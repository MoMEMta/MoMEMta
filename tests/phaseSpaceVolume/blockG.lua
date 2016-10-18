function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end

function copy_and_append(t1, t2)
    local t3 = {}

    append(t3, t1)
    append(t3, t2)

    return t3
end

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
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- Pass these outputs over for Phi
FlatTransferFunctionOnPhi.tf_phi_1 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/1',
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
    inputs = inputs,

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
