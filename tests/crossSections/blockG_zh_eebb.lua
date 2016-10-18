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
    n_start = 100000,   
    n_increase = 100000,
    seed = 5468460,        
}

-- 'Flat' transfer functions to integrate over the visible particle's angles

-- First do Phi
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

BreitWignerGenerator.flatter_z = {
    ps_point = add_dimension(),
    mass = z_mass,
    width = z_width
}

BreitWignerGenerator.flatter_h = {
    ps_point = add_dimension(),
    mass = h_mass,
    width = h_width
}

BlockG.blockg = {
    inputs = inputs,

    s12 = 'flatter_z::s',
    s34 = 'flatter_h::s',
}

-- Loop

Looper.looper = {
    solutions = "blockg::solutions",
    path = Path("initial_state", "me_zh", "integrand")
}

    gen_inputs = {'looper::particles/1', 'looper::particles/2', 'looper::particles/3', 'looper::particles/4'}
    
    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian', 'tf_phi_4::TF_times_jacobian', 
      'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian', 'tf_theta_4::TF_times_jacobian', 
      'flatter_z::jacobian', 'flatter_h::jacobian',
      'looper::jacobian',
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
