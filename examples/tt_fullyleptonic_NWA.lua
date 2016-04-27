-- Use transfer functions
inputs_before_perm = {
    'tf_p1::output',
    'tf_p2::output',
    'tf_p3::output',
    'tf_p4::output',
}

-- Use permutator module to permutate input particles 0 and 2 using the MC
inputs = {
  inputs_before_perm[1],
  'permutator::output/0',
  inputs_before_perm[3],
  'permutator::output/1',
}

configuration = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600,
}

cuba = {
    verbosity = 3
}

-- Use the narrow width approximation for both Top & W propagators
NarrowWidthApproximation.nwa_s13 = {
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

NarrowWidthApproximation.nwa_s134 = {
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

NarrowWidthApproximation.nwa_s25 = {
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

NarrowWidthApproximation.nwa_s256 = {
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

GaussianTransferFunction.tf_p1 = {
    ps_point = 'cuba::ps_points/0',
    reco_particle = 'input::particles/0',
    sigma = 0.05,
}

GaussianTransferFunction.tf_p2 = {
    ps_point = 'cuba::ps_points/1',
    reco_particle = 'input::particles/1',
    sigma = 0.10,
}

GaussianTransferFunction.tf_p3 = {
    ps_point = 'cuba::ps_points/2',
    reco_particle = 'input::particles/2',
    sigma = 0.05,
}

GaussianTransferFunction.tf_p4 = {
    ps_point = 'cuba::ps_points/3',
    reco_particle = 'input::particles/3',
    sigma = 0.10,
}
  
Permutator.permutator = {
    ps_point = 'cuba::ps_points/4',
    input = {
      inputs_before_perm[2],
      inputs_before_perm[4],
    }
}

BlockD.blockd = {
    inputs = inputs,

    s13 = 'nwa_s13::s',
    s134 = 'nwa_s134::s',
    s25 = 'nwa_s25::s',
    s256 = 'nwa_s256::s',
}

Boost.boost = {
    invisibles = {
        'blockd::invisibles',
    },

    particles = inputs
}


jacobians = {
  'nwa_s13::jacobian', 'nwa_s134::jacobian', 'nwa_s25::jacobian', 'nwa_s256::jacobian', 
}

MatrixElement.ttbar = {
  pdf = 'CT10nlo',

  matrix_element = 'pp_ttx_fully_leptonic',
  matrix_element_parameters = {
      card = '../MatrixElements/Cards/param_card.dat'
  },

  initialState = 'boost::output',

  invisibles = {
    input = 'blockd::invisibles',
    jacobians = 'blockd::jacobians',
    ids = {
      {
        pdg_id = 12,
        me_index = 2,
      },

      {
        pdg_id = -14,
        me_index = 5,
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
        pdg_id = 5,
        me_index = 3,
      },

      {
        pdg_id = 13,
        me_index = 4,
      },

      {
        pdg_id = -5,
        me_index = 6,
      },
    }
  },

  jacobians = jacobians
}
