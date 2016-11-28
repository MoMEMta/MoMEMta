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
    max_eval = 1000000000,
    relative_accuracy = 0.005,
    n_start = 100000000,
    n_increase = 100000000,
    seed = 5468460,
    ncores = 2,
    batch_size = 1000000
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- First |P|
FlatTransferFunctionOnP.tf_p_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}
p3.set_gen_p4("tf_p_3::output")

FlatTransferFunctionOnP.tf_p_4 = {
    ps_point = add_dimension(),
    reco_particle = p4.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}
p4.set_gen_p4("tf_p_4::output")

-- Then Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.reco_p4,
}
p1.set_gen_p4("tf_phi_1::output")

FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = p2.reco_p4,
}
p2.set_gen_p4("tf_phi_2::output")

FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.gen_p4,
}
p3.set_gen_p4("tf_phi_3::output")

FlatTransferFunctionOnPhi.tf_phi_4 = {
    ps_point = add_dimension(),
    reco_particle = p4.gen_p4,
}
p4.set_gen_p4("tf_phi_4::output")

-- Finally, do Theta
FlatTransferFunctionOnTheta.tf_theta_1 = {
    ps_point = add_dimension(),
    reco_particle = p1.gen_p4,
}
p1.set_gen_p4('tf_phi_1::output')

FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = add_dimension(),
    reco_particle = p2.gen_p4,
}
p2.set_gen_p4('tf_phi_2::output')

FlatTransferFunctionOnTheta.tf_theta_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.gen_p4,
}
p3.set_gen_p4('tf_phi_3::output')

FlatTransferFunctionOnTheta.tf_theta_4 = {
    ps_point = add_dimension(),
    reco_particle = p4.gen_p4,
}
p4.set_gen_p4('tf_phi_4::output')

inputs = {
  p1.gen_p4,
  p2.gen_p4,
  p3.gen_p4,
  p4.gen_p4,
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = { inputs[3], inputs[4] }
}

BlockA.blocka = {
    p1 = inputs[1],
    p2 = inputs[2],
    p3 = inputs[3],
    p4 = inputs[4],
}

-- Loop

Looper.looper = {
    solutions = "blocka::solutions",
    path = Path("initial_state", "me_ww", "integrand")
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
