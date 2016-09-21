parameters = {
    energy = 13000.,
    top_mass = 173.
}

inputs = {
    'input::particles/1',
    'input::particles/2',
    'input::particles/3',
    'input::particles/4',
    'input::particles/5',
    'input::particles/6',
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = {'input::particles/1', 'input::particles/2', 'input::particles/3', 'input::particles/4'} -- only on visible particles
}

BuildInitialState.boost = {
    do_transverse_boost = true,
    particles = inputs
}

MatrixElement.ttbar = {
    pdf = 'CT10nlo',
    pdf_scale = parameter('top_mass'),
    matrix_element = 'pp_ttx_fully_leptonic',
    matrix_element_parameters = {
        card = '../../MatrixElements/Cards/param_card.dat'
    },
    initialState = 'boost::partons',
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
    jacobians = {'phaseSpaceOut::phase_space'}
}

integrand("ttbar::output")
