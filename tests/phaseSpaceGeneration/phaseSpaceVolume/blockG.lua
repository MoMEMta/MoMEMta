local p1 = declare_input("p1")
local p2 = declare_input("p2")
local p3 = declare_input("p3")
local p4 = declare_input("p4")

load_modules('MatrixElements/dummy/libme_dummy.so')

Mass = 250.
Width = 500.

parameters = {
    energy = 1000.,
}

cuba = {
    verbosity = 3,
    max_eval = 200000000,
    relative_accuracy = 0.001,
    n_start = 20000000,
    seed = 5468460,
    ncores = 8,
    batch_size = 8000000
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- Pass these outputs over for Phi
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

BreitWignerGenerator.flatter_s12 = {
    ps_point = add_dimension(),
    mass = Mass,
    width = Width
}

BreitWignerGenerator.flatter_s34 = {
    ps_point = add_dimension(),
    mass = Mass,
    width = Width
}

BlockG.blockg = {
    p1 = inputs[1],
    p2 = inputs[2],
    p3 = inputs[3],
    p4 = inputs[4],

    s12 = 'flatter_s12::s',
    s34 = 'flatter_s34::s',
}

-- Loop

Looper.looper = {
    solutions = "blockg::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    gen_inputs = {'looper::particles/1', 'looper::particles/2', 'looper::particles/3', 'looper::particles/4'}

    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian',
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian',
      'flatter_s12::jacobian', 'flatter_s34::jacobian',
      'looper::jacobian',
    }

    MatrixElement.dummy = {
      use_pdf = false,

      matrix_element = 'dummy_matrix_element',
      matrix_element_parameters = {},

      initialState = 'initial_state::partons',

      particles = {
        inputs = gen_inputs,
        ids = {
          {
            pdg_id = -11,
            me_index = 1,
          },

          {
            pdg_id = 5,
            me_index = 3,
          },

          {
            pdg_id = 11,
            me_index = 2,
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
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
