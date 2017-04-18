local p1 = declare_input("p1")
local p3 = declare_input("p3")

load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    
    s13_mass = 100.,
    s13_width = 100.,
    
    shat_mass = 200.,
    shat_width = 200.,
}

cuba = {
    verbosity = 3,
    max_eval = 1000000000,
    relative_accuracy = 0.0005,
    n_start = 200000000,
    n_increase = 0,
    seed = 0,
    ncores = 10,
    batch_size = 10000000
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|
FlatTransferFunctionOnP.tf_p_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_1::output',
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_3::output',
}

-- Finally, do Theta
FlatTransferFunctionOnTheta.tf_theta_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_1::output',
}
FlatTransferFunctionOnTheta.tf_theta_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_3::output',
}

inputs = {
  'tf_theta_1::output',
  'tf_theta_3::output',
}

BreitWignerGenerator.flatter_s13 = {
    ps_point = add_dimension(),
    mass = parameter('s13_mass'),
    width = parameter('s13_width')
}

BreitWignerGenerator.flatter_s24 = {
    ps_point = add_dimension(),
    mass = parameter('s13_mass'),
    width = parameter('s13_width')
}

BreitWignerGenerator.flatter_shat = {
    ps_point = add_dimension(),
    mass = parameter('shat_mass'),
    width = parameter('shat_width')
}

UniformGenerator.rapidity = {
    ps_point = add_dimension(),
    min = -100.,
    max = 100.
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs
}

BlockE.blocke = {
    p3 = inputs[1],
    p4 = inputs[2],

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    s_hat = 'flatter_shat::s',
    y_tot = 'rapidity::output',
}

-- Loop

Looper.looper = {
    solutions = "blocke::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

    BuildInitialState.initial_state = {
        particles = full_inputs
    }

    jacobians = {
      'tf_p_1::TF_times_jacobian', 'tf_p_3::TF_times_jacobian',
      'tf_phi_1::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian',
      'flatter_s13::jacobian', 'flatter_s24::jacobian', 'flatter_shat::jacobian', 'rapidity::jacobian',
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
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
