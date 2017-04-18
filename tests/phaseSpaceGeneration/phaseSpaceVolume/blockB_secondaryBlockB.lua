local p1 = declare_input("p1")
local p2 = declare_input("p2")
local p3 = declare_input("p3")

load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    mass_12 = 250.,
    width_12 = 250.,
    mass_123 = 500.,
    width_123 = 500.,
    mass_B = 1000.,
    width_B = 1000.,
}

cuba = {
    verbosity = 3,
    max_eval = 1000000000,
    relative_accuracy = 0.0005,
    n_start = 50000000,
    n_increase = 10000000,
    seed = 468960,
    ncores = 10,
    batch_size = 10000000
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|: only two inputs to the secondary block are concerned
FlatTransferFunctionOnP.tf_p_2 = {
    ps_point = add_dimension(),
    reco_particle = p2.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi: the inputs of the secondary block are integrated over their angles
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.reco_p4,
}
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_2::output',
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_3::output',
}

-- Finally, do Theta, except for the first input to the secondary block
FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_2::output',
}
FlatTransferFunctionOnTheta.tf_theta_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_3::output',
}

inputs = {
  'tf_phi_1::output',
  'tf_theta_2::output',
  'tf_theta_3::output',
}

-- Only compute phase-space on particle not concerned by the blocks, ie. only one input to the secondary block
StandardPhaseSpace.phaseSpaceOut = {
    particles = { inputs[2], inputs[3] }
}

BreitWignerGenerator.flatter_s12 = {
    ps_point = add_dimension(),
    mass = parameter('mass_12'),
    width = parameter('width_12')
}
BreitWignerGenerator.flatter_s123 = {
    ps_point = add_dimension(),
    mass = parameter('mass_123'),
    width = parameter('width_123')
}
BreitWignerGenerator.flatter_sB = {
    ps_point = add_dimension(),
    mass = parameter('mass_B'),
    width = parameter('width_B')
}


SecondaryBlockB.secBlockB = {
    s12 = 'flatter_s12::s',
    s123 = 'flatter_s123::s',
    p1 = inputs[1],
    p2 = inputs[2],
    p3 = inputs[3],
}

Looper.looperSBB = {
    solutions = 'secBlockB::solutions',
    path = Path('sum_SBB', 'blockb', 'looperB')
}

-- Loop for secondary block

    VectorLinearCombinator.sum_SBB = {
        coefficients = { 1., 1., 1. },
        inputs = { 'looperSBB::particles/1', inputs[2], inputs[3] }
    }

    BlockB.blockb = {
        s12 = 'flatter_sB::s',
        p2 = 'sum_SBB::output',
    }

    Looper.looperB = {
        solutions = "blockb::solutions",
        path = Path("initial_state", "dummy", "integrand")
    }

    -- Loop for main block

        full_inputs = { 'looperB::particles/1', 'looperSBB::particles/1', inputs[2], inputs[3] }

        BuildInitialState.initial_state = {
            particles = full_inputs
        }

        jacobians = {
          'tf_p_2::TF_times_jacobian', 'tf_p_3::TF_times_jacobian',
          'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian',
          'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian',
          'flatter_s12::jacobian', 'flatter_s123::jacobian', 'flatter_sB::jacobian', 'looperSBB::jacobian', 'looperB::jacobian', 'phaseSpaceOut::phase_space'
        }

        MatrixElement.dummy = {
          use_pdf = false,

          matrix_element = 'dummy_matrix_element',
          matrix_element_parameters = {},

          initialState = 'initial_state::partons',

          particles = {
            inputs = full_inputs,
            -- Dummy IDs since no real matrix element
            -- However me_index should be valid (ie not twice the same)
            ids = {
                { pdg_id = 1, me_index = 1 },
                { pdg_id = 1, me_index = 2 },
                { pdg_id = 1, me_index = 3 },
                { pdg_id = 1, me_index = 4 },
            }
          },

          jacobians = jacobians
        }

        DoubleLooperSummer.integrand = {
            input = "dummy::output"
        }

-- End of loops

integrand("integrand::sum")
