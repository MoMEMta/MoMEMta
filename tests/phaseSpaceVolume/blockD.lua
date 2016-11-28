local p1 = declare_input("p1")
local p2 = declare_input("p2")
local p3 = declare_input("p3")
local p4 = declare_input("p4")

load_modules('MatrixElements/dummy/libme_dummy.so')

M_W = 250.
M_TOP = 500.
W_W = 100.
W_TOP = 100.

parameters = {
    energy = 1000.,
    top_mass = M_TOP,
    W_mass = M_W
}

cuba = {
    verbosity = 3,
    max_eval = 200000000000,
    relative_accuracy = 0.005,
    n_start    = 5000000,
    n_increase = 1000000,
    seed = 5468460,
    ncores = 8,
    batch_size = 8000000
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|
FlatTransferFunctionOnP.tf_p_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}
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

-- Pass these outputs over for Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_1::output',
}
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
FlatTransferFunctionOnTheta.tf_theta_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_1::output',
}
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
  'tf_theta_1::output',
  'tf_theta_2::output',
  'tf_theta_3::output',
  'tf_theta_4::output',
}

BreitWignerGenerator.flatter_s13 = {
    ps_point = add_dimension(),
    mass = M_W,
    width = W_W
}

BreitWignerGenerator.flatter_s134 = {
    ps_point = add_dimension(),
    mass = M_TOP,
    width = W_TOP
}

BreitWignerGenerator.flatter_s25 = {
    ps_point = add_dimension(),
    mass = M_W,
    width = W_W
}

BreitWignerGenerator.flatter_s256 = {
    ps_point = add_dimension(),
    mass = M_TOP,
    width = W_TOP
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs
}

BlockD.blockd = {
    p3 = inputs[1],
    p4 = inputs[2],
    p5 = inputs[3],
    p6 = inputs[4],

    s13 = 'flatter_s13::s',
    s134 = 'flatter_s134::s',
    s25 = 'flatter_s25::s',
    s256 = 'flatter_s256::s',
}

-- Loop

Looper.looper = {
    solutions = "blockd::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

    BuildInitialState.initial_state = {
        particles = full_inputs
    }

    jacobians = {
      'tf_p_1::TF_times_jacobian', 'tf_p_2::TF_times_jacobian', 'tf_p_3::TF_times_jacobian', 'tf_p_4::TF_times_jacobian',
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian',
      'flatter_s13::jacobian', 'flatter_s134::jacobian', 'flatter_s25::jacobian', 'flatter_s256::jacobian',
      'looper::jacobian', 'phaseSpaceOut::phase_space'
    }

    MatrixElement.dummy = {
      use_pdf = false,

      matrix_element = 'dummy_matrix_element',
      matrix_element_parameters = {},

      initialState = 'initial_state::partons',

      particles = {
        inputs = full_inputs,
        ids = {
            { pdg_id = 1, me_index = 1 },
            { pdg_id = 1, me_index = 2 },
            { pdg_id = 1, me_index = 3 },
            { pdg_id = 1, me_index = 4 },
            { pdg_id = 1, me_index = 5 },
            { pdg_id = 1, me_index = 6 },
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
