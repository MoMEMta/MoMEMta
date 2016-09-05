parameters = {
    energy = 13000.,
    W_mass = 80.419002,
    W_width = 2.047600e+00
}

cuba = {
    verbosity = 2,
    max_eval = 20000000000,
    relative_accuracy = 0.005,
    n_start = 1000000000,   
    seed = 5468730,        
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|
FlatTransferFunctionOnP.tf_p_1 = {
    ps_point = getpspoint(),
    reco_particle = 'input::particles/1',
    min = 0.,
    max = parameters.energy/2,
}
FlatTransferFunctionOnP.tf_p_2 = {
    ps_point = getpspoint(),
    reco_particle = 'input::particles/2',
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = getpspoint(),
    reco_particle = 'tf_p_1::output',
}
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = getpspoint(),
    reco_particle = 'tf_p_2::output',
}

-- Finally, do Theta 
FlatTransferFunctionOnTheta.tf_theta_1 = {
    ps_point = getpspoint(),
    reco_particle = 'tf_phi_1::output',
}
FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = getpspoint(),
    reco_particle = 'tf_phi_2::output',
}

inputs = {
  'tf_theta_1::output',
  'tf_theta_2::output',
}

BreitWignerGenerator.flatter_s13 = {
    ps_point = getpspoint(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BreitWignerGenerator.flatter_s24 = {
    ps_point = getpspoint(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BlockF.blockf = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    q1 = getpspoint(),
    q2 = getpspoint()
}

-- Loop

Looper.looper = {
    solutions = "blockf::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    BuildInitialState.initial_state = {
        solution = 'looper::solution',
        particles = inputs
    }

    jacobians = {
      'tf_p_1::TF_times_jacobian', 'tf_p_2::TF_times_jacobian', 
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian',
      'flatter_s13::jacobian', 'flatter_s24::jacobian',
    }

    MatrixElement.dummy = {
      pdf = 'CT10nlo',
      pdf_scale = parameter('W_mass'),

      matrix_element = 'pp_WW_fully_leptonic_sm_P1_Sigma_sm_uux_epvemumvmx',
      matrix_element_parameters = {
          card = '../MatrixElements/Cards/param_card.dat'
      },

      initialState = 'initial_state::partons',

      invisibles = {
        input = 'looper::solution',
        ids = {
          {
            pdg_id = 12,
            me_index = 2,
          },

          {
            pdg_id = -14,
            me_index = 4,
          }
        }
      },

      particles = {
        inputs = inputs,
        ids = {
          {
            pdg_id = -11,
            me_index = 1,
          },

          {
            pdg_id = 13,
            me_index = 3,
          },
        }
      },

      jacobians = jacobians
    }

    DoubleSummer.integrand = {
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
