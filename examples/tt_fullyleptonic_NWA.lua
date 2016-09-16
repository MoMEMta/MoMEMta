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
  'permutator::output/1',
  inputs_before_perm[3],
  'permutator::output/2',
}

parameters = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600,
}

cuba = {
    relative_accuracy = 0.01,
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

GaussianTransferFunctionOnEnergy.tf_p1 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/1',
    sigma = 0.05,
}

GaussianTransferFunctionOnEnergy.tf_p2 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/2',
    sigma = 0.10,
}

GaussianTransferFunctionOnEnergy.tf_p3 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/3',
    sigma = 0.05,
}

GaussianTransferFunctionOnEnergy.tf_p4 = {
    ps_point = add_dimension(),
    reco_particle = 'input::particles/4',
    sigma = 0.10,
}
  
Permutator.permutator = {
    ps_point = add_dimension(),
    inputs = {
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

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs -- only on visible particles
}

-- Loop over block solutions
Looper.looper = {
    solutions = "blockd::solutions",
    path = Path("initial_state", "ttbar", "integrand")
}

full_inputs = {
    inputs_before_perm[1],
    'permutator::output/1',
    inputs_before_perm[3],
    'permutator::output/2',
    'looper::particles/1',
    'looper::particles/2',
}

BuildInitialState.initial_state = {
    solution = 'looper::solution',
    particles = full_inputs
}


jacobians = {
  'nwa_s13::jacobian', 'nwa_s134::jacobian', 'nwa_s25::jacobian', 'nwa_s256::jacobian',
  'looper::jacobian', 'phaseSpaceOut::phase_space'
}

MatrixElement.ttbar = {
  pdf = 'CT10nlo',
  pdf_scale = parameter('top_mass'),

  matrix_element = 'pp_ttx_fully_leptonic',
  matrix_element_parameters = {
      card = '../MatrixElements/Cards/param_card.dat'
  },

  initialState = 'initial_state::partons',

  particles = {
    inputs = full_inputs,
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

  jacobians = jacobians
}
    DoubleSummer.integrand = {
        input = "ttbar::output"
    }

-- End of loop
integrand("integrand::sum")
