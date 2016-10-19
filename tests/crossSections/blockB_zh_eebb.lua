z_mass = 91.1188
z_width = 2.441404
h_mass = 125.0
h_width = 0.006382339

parameters = {
    energy = 13000.,
}

cuba = {
    verbosity = 3,
    max_eval = 200000000,
    relative_accuracy = 0.005,
    n_start = 1000000,   
    n_increase = 1000000,
    seed = 5468460,        
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- First |P|
FlatTransferFunctionOnP.tf_p_2 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/2',
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_3 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/3',
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_4 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/4',
    min = 0.,
    max = parameters.energy/2,
}

-- Then Phi
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

BreitWignerGenerator.flatter_h = {
    ps_point = add_dimension(),
    mass = h_mass,
    width = h_width
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = { inputs[2], inputs[3], inputs[4] }
}

BlockB.blockb = {
    s12 = 'flatter_h::s',
    inputs = inputs,
}

-- Loop

Looper.looper = {
    solutions = "blocka::solutions",
    path = Path("initial_state", "me_zh", "integrand")
}

    gen_inputs = { 'looper::particles/1', 'looper::particles/2', inputs[3], inputs[4] }
    
    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_p_3::TF_times_jacobian', 'tf_p_4::TF_times_jacobian', 
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian', 
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian', 
      'phaseSpaceOut::phase_space', 'looper::jacobian',
    }

    MatrixElement.me_zh = {
      pdf = 'CT10nlo',
      pdf_scale = z_mass + h_mass,

      matrix_element = 'pp_zh_z_ee_h_bb_sm',
      matrix_element_parameters = {
          card = '../MatrixElements/Cards/param_card_sm_5fs.dat'
      },

      initialState = 'initial_state::partons',

      particles = {
        inputs = gen_inputs,
        ids = {
          {
            pdg_id = -11,
            me_index = 1,
          },

          {
            pdg_id = 11,
            me_index = 2,
          },

          {
            pdg_id = 5,
            me_index = 3,
          },

          {
            pdg_id = -5,
            me_index = 4,
          },
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "me_zh::output"
    }

-- End of loop

integrand("integrand::sum")
