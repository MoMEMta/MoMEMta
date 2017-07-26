-- Register inputs
local electron = declare_input("electron")
local muon = declare_input("muon")
local bjet1 = declare_input("bjet1")
local bjet2 = declare_input("bjet2")

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
    reco_particle = electron.reco_p4,
    sigma = 0.05,
}
electron.set_gen_p4("tf_p1::output")

GaussianTransferFunctionOnEnergy.tf_p2 = {
    ps_point = add_dimension(),
    reco_particle = bjet1.reco_p4,
    sigma = 0.10,
}
bjet1.set_gen_p4("tf_p2::output")

GaussianTransferFunctionOnEnergy.tf_p3 = {
    ps_point = add_dimension(),
    reco_particle = muon.reco_p4,
    sigma = 0.05,
}
muon.set_gen_p4("tf_p3::output")

GaussianTransferFunctionOnEnergy.tf_p4 = {
    ps_point = add_dimension(),
    reco_particle = bjet2.reco_p4,
    sigma = 0.10,
}
bjet2.set_gen_p4("tf_p4::output")

-- Enable permutations between the two b-jets (gen level only)
add_gen_permutations(bjet1, bjet2)

-- If set_gen_p4 is not called, gen_p4 == reco_p4
inputs = {
    electron.gen_p4,
    bjet1.gen_p4,
    muon.gen_p4,
    bjet2.gen_p4
}

BlockD.blockd = {
    p3 = inputs[1],
    p4 = inputs[2],
    p5 = inputs[3],
    p6 = inputs[4],

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

full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

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
        pdg_id = -13,
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
        pdg_id = 14,
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
    DoubleLooperSummer.integrand = {
        input = "ttbar::output"
    }

-- End of loop
integrand("integrand::sum")
