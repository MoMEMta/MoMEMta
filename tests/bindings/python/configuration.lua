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
    verbosity = 0,
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

add_gen_permutations(bjet1, bjet2)

inputs = {
    electron.gen_p4,
    bjet1.gen_p4,
    muon.gen_p4,
    bjet2.gen_p4
}

-- Declare module before the permutator to test read-access in the pool
-- for non-existant values.
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

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs -- only on visible particles
}

-- Loop over block solutions

Looper.looper = {
    solutions = "blockd::solutions",
    path = Path("boost", "ttbar", "integrand")
}

full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

BuildInitialState.boost = {
    do_transverse_boost = true,
    particles = full_inputs
}

jacobians = {
    'flatter_s13::jacobian', 'flatter_s134::jacobian', 'flatter_s25::jacobian', 'flatter_s256::jacobian',
    'tf_p1::TF_times_jacobian', 'tf_p2::TF_times_jacobian', 'tf_p3::TF_times_jacobian', 'tf_p4::TF_times_jacobian',
    'looper::jacobian', 'phaseSpaceOut::phase_space'
}

MatrixElement.ttbar = {
    pdf = 'CT10nlo',
    pdf_scale = parameter('top_mass'),
    matrix_element = 'pp_ttx_fully_leptonic',
    matrix_element_parameters = {
        card = '../MatrixElements/Cards/param_card.dat'
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

-- End of loop
integrand("integrand::sum")
