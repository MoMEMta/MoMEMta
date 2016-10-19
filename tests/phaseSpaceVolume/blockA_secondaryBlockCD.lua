load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    mass = 500.,
    width = 250.
}

cuba = {
    verbosity = 3,
    max_eval = 20000000,
    relative_accuracy = 0.001,
    n_start = 2000000,   
    n_increase = 1000000,   
    seed = 5468960,
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|: only the first input is integrated over
FlatTransferFunctionOnP.tf_p_1 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/1',
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi: the two outputs of blockA and the output of the secondary block are integrated over their angles
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_1::output',
}
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/2',
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/3',
}
FlatTransferFunctionOnPhi.tf_phi_4 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/4',
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

-- Only compute phase-space on particle not concerned by the blocks
StandardPhaseSpace.phaseSpaceOut = {
    particles = { inputs[1] }
}

BreitWignerGenerator.flatter = {
    ps_point = add_dimension(),
    mass = parameter('mass'),
    width = parameter('width')
}

SecondaryBlockCD.secBlockCD = {
    s12 = 'flatter::s',
    gen_p2 = inputs[1],
    reco_p1 = inputs[2],
}

Looper.looperCD = {
    solutions = 'secBlockCD::gen_p1',
    path = Path('blocka', 'looperA')
}

-- Loop for secondary block

    BlockA.blocka = {
        inputs = { inputs[3], inputs[4], inputs[1], 'looperCD::particles/1' }
    }

    Looper.looperA = {
        solutions = "blocka::solutions",
        path = Path("initial_state", "dummy", "integrand")
    }
    
    -- Loop for main block
    
        full_inputs = { 'looperA::particles/1', 'looperA::particles/2', inputs[1], 'looperCD::particles/1' }
    
        BuildInitialState.initial_state = {
            particles = full_inputs
        }
    
        jacobians = {
          'tf_p_1::TF_times_jacobian',
          'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian', 
          'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian', 
          'flatter::jacobian', 'looperCD::jacobian', 'looperA::jacobian', 'phaseSpaceOut::phase_space'
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
