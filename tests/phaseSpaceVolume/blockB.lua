load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    W_mass = 500.,
    W_width = 500.
}

cuba = {
    verbosity = 3,
    max_eval = 200000000,
    relative_accuracy = 0.001,
    n_start = 20000000,   
    seed = 5468960,        
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|
FlatTransferFunctionOnP.tf_p_1 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/1',
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_2 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/2',
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

-- Finally, do Theta 
FlatTransferFunctionOnTheta.tf_theta_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_1::output',
}
FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_2::output',
}

inputs = {
  'tf_theta_1::output',
  'tf_theta_2::output',
}

BreitWignerGenerator.flatter = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs
}

BlockB.blockb = {
    inputs = inputs,
    s12 = 'flatter::s',
}

-- Loop

Looper.looper = {
    solutions = "blockb::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    gen_inputs = { inputs[1], inputs[2], 'looper::particles/1' }

    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_p_1::TF_times_jacobian', 'tf_p_2::TF_times_jacobian',
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian',
      'flatter::jacobian',
      'looper::jacobian', 'phaseSpaceOut::phase_space'
    }

    MatrixElement.dummy = {
      use_pdf = false,

      matrix_element = 'dummy_matrix_element',
      matrix_element_parameters = {},
 
      initialState = 'initial_state::partons',

      particles = {
        inputs = gen_inputs,
        ids = {
            { pdg_id = 1, me_index = 1 },
            { pdg_id = 1, me_index = 2 },
            { pdg_id = 1, me_index = 3 },
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
