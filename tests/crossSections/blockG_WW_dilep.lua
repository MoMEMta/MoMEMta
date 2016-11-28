local p1 = declare_input("p1")
local p2 = declare_input("p2")
local p3 = declare_input("p3")
local p4 = declare_input("p4")

parameters = {
    energy = 13000.,
    W_mass = 80.419002,
    W_width = 2.0476
}

cuba = {
    verbosity = 3,
    max_eval = 200000000,
    relative_accuracy = 0.005,
    n_start = 100000,
    n_increase = 100000,
    seed = 5468460,
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- First do Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.reco_p4,
}
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = p2.reco_p4,
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.reco_p4,
}
FlatTransferFunctionOnPhi.tf_phi_4 = {
    ps_point = add_dimension(),
    reco_particle = p4.reco_p4,
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

BreitWignerGenerator.flatter_w1 = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width'),
}

BreitWignerGenerator.flatter_w2 = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width'),
}

BlockG.blockg = {
    p1 = inputs[1],
    p2 = inputs[2],
    p3 = inputs[3],
    p4 = inputs[4],

    s12 = 'flatter_w1::s',
    s34 = 'flatter_w2::s',
}

-- Loop

Looper.looper = {
    solutions = "blockg::solutions",
    path = Path("initial_state", "me_ww", "integrand")
}

    gen_inputs = {'looper::particles/1', 'looper::particles/2', 'looper::particles/3', 'looper::particles/4'}

    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian',
      'flatter_w1::jacobian', 'flatter_w2::jacobian',
      'looper::jacobian',
    }

    MatrixElement.me_ww = {
      pdf = 'CT10nlo',
      pdf_scale = parameter('W_mass'),

      matrix_element = 'pp_WW_fully_leptonic_sm_P1_Sigma_sm_uux_epvemumvmx',
      matrix_element_parameters = {
          card = '../MatrixElements/Cards/param_card.dat'
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
            pdg_id = 12,
            me_index = 2,
          },

          {
            pdg_id = 13,
            me_index = 3,
          },

          {
            pdg_id = -14,
            me_index = 4,
          },
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "me_ww::output"
    }

-- End of loop

integrand("integrand::sum")
