local p1 = declare_input("p1")
local p2 = declare_input("p2")
local p3 = declare_input("p3")
local p4 = declare_input("p4")

load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    
    mass_12 = 100.,
    width_12 = 100.,
    
    mass_123 = 250.,
    width_123 = 250.,
    
    mass_1234 = 500.,
    width_1234 = 500.,
    
    mass_B = 1000.,
    width_B = 1000.,
}

cuba = {
    verbosity = 3,
    max_eval = 1000000000,
    relative_accuracy = 0.0005,
    n_start = 100000000,
    n_increase = 0,
    seed = 0,
    ncores = 10,
    batch_size = 10000000
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|: the three inputs to the secondary block are concerned
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
FlatTransferFunctionOnP.tf_p_4 = {
    ps_point = add_dimension(),
    reco_particle = p4.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi: the inputs of the secondary block are integrated over their angles
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_2::output',
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_3::output',
}
FlatTransferFunctionOnPhi.tf_phi_4 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_4::output',
}

-- Finally, do Theta
FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_2::output',
}
FlatTransferFunctionOnTheta.tf_theta_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_3::output',
}
FlatTransferFunctionOnTheta.tf_theta_4 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_4::output',
}

inputs = {
  'tf_theta_2::output',
  'tf_theta_3::output',
  'tf_theta_4::output',
}

-- Only compute phase-space on particle not concerned by the blocks, ie. the inputs to the secondary block
StandardPhaseSpace.phaseSpaceOut = {
    particles = { inputs[1], inputs[2], inputs[3] }
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
BreitWignerGenerator.flatter_s1234 = {
    ps_point = add_dimension(),
    mass = parameter('mass_1234'),
    width = parameter('width_1234')
}
BreitWignerGenerator.flatter_sB = {
    ps_point = add_dimension(),
    mass = parameter('mass_B'),
    width = parameter('width_B')
}


SecondaryBlockA.secBlockA = {
    s12 = 'flatter_s12::s',
    s123 = 'flatter_s123::s',
    s1234 = 'flatter_s1234::s',
    p1 = p1.reco_p4,
    p2 = inputs[1],
    p3 = inputs[2],
    p4 = inputs[3]
}

Looper.looperSBA = {
    solutions = 'secBlockA::solutions',
    path = Path('sum_SBA', 'blockb', 'looperB')
}

-- Loop for secondary block

    VectorLinearCombinator.sum_SBA = {
        coefficients = { 1., 1., 1., 1. },
        inputs = { inputs[1], inputs[2], inputs[3], 'looperSBA::particles/1' }
    }

    BlockB.blockb = {
        s12 = 'flatter_sB::s',
        p2 = 'sum_SBA::output',
    }

    Looper.looperB = {
        solutions = "blockb::solutions",
        path = Path("initial_state", "dummy", "integrand")
    }

    -- Loop for main block

        full_inputs = { 'looperB::particles/1', inputs[1], inputs[2], inputs[3], 'looperSBA::particles/1' }

        BuildInitialState.initial_state = {
            particles = full_inputs
        }

        jacobians = {
          'tf_p_2::TF_times_jacobian', 'tf_p_3::TF_times_jacobian', 'tf_p_4::TF_times_jacobian',
          'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian',
          'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian',
          'flatter_s12::jacobian', 'flatter_s123::jacobian', 'flatter_s1234::jacobian', 'flatter_sB::jacobian', 'looperSBA::jacobian', 'looperB::jacobian', 'phaseSpaceOut::phase_space'
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
                { pdg_id = 1, me_index = 5 },
            }
          },

          jacobians = jacobians
        }

        DoubleLooperSummer.integrand = {
            input = "dummy::output"
        }

-- End of loops

integrand("integrand::sum")
