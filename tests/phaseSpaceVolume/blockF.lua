load_modules('MatrixElements/dummy/libme_dummy.so')

M_W = 250.
M_TOP = 500.
W_W = 100.
W_TOP = 100.

parameters = {
    energy = 1000.,
    top_mass = M_W,
    W_mass = M_W
}

cuba = {
    verbosity = 2,
    max_eval = 20000000000,
    relative_accuracy = 0.005,
    n_start = 1000000000,   
    seed = 5468960,        
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
    mass = M_W,
    width = W_W
}

BreitWignerGenerator.flatter_s24 = {
    ps_point = getpspoint(),
    mass = M_W,
    width = W_W
}

BlockF.blockf = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    q1 = getpspoint(),
    q2 = getpspoint()
}

BuildInitialState.initial_state = {
    invisibles = {
        'blockf::invisibles',
    },

    particles = inputs
}

jacobians = {
  'tf_p_1::TF_times_jacobian', 'tf_p_2::TF_times_jacobian', 
  'tf_phi_1::TF_times_jacobian', 'tf_phi_2::TF_times_jacobian', 
  'tf_theta_1::TF_times_jacobian', 'tf_theta_2::TF_times_jacobian', 
  'flatter_s13::jacobian', 'flatter_s24::jacobian'
}

MatrixElement.dummy = {
  use_pdf = false,

  matrix_element = 'dummy_matrix_element',
  matrix_element_parameters = {},
  
  initialState = 'initial_state::output',

  invisibles = {
    input = 'blockf::invisibles',
    jacobians = 'blockf::jacobians',
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
