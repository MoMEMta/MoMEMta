local electron = declare_input("electron")
local muon = declare_input("muon")
local bjet1 = declare_input("bjet1")
local bjet2 = declare_input("bjet2")

parameters = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600
}

BreitWignerGenerator.flatter_s13 = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BreitWignerGenerator.flatter_s134 = {
    ps_point = add_dimension(),
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

BreitWignerGenerator.flatter_s25 = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BreitWignerGenerator.flatter_s256 = {
    ps_point = add_dimension(),
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

inputs = {
    electron.reco_p4,
    bjet1.reco_p4,
    muon.reco_p4,
    bjet2.reco_p4
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs
}

BlockD.blockd = {
    p3 = inputs[1],
    p4 = inputs[2],
    p5 = inputs[3],
    p6 = inputs[4],

    pT_is_met = true,

    s13 = 'flatter_s13::s',
    s134 = 'flatter_s134::s',
    s25 = 'flatter_s25::s',
    s256 = 'flatter_s256::s',
}

Looper.looper = {
    solutions = "blockd::solutions",
    path = Path("boost", "ttbar", "integrand")
}

    full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

    BuildInitialState.boost = {
        do_transverse_boost = true,
        particles = full_inputs
    }

    jacobians = {'flatter_s13::jacobian', 'flatter_s134::jacobian', 'flatter_s25::jacobian', 'flatter_s256::jacobian'}

    append(jacobians, {'phaseSpaceOut::phase_space', 'looper::jacobian'})

    MatrixElement.ttbar = {
      pdf = 'CT10nlo',
      pdf_scale = parameter('top_mass'),

      matrix_element = 'pp_ttx_fully_leptonic',
      matrix_element_parameters = {
          card = '../../MatrixElements/Cards/param_card.dat',
      },

      initialState = 'boost::partons',

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

integrand("integrand::sum")
