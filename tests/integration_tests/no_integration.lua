local electron = declare_input("electron")
local muon = declare_input("muon")
local bjet1 = declare_input("bjet1")
local bjet2 = declare_input("bjet2")
local neutrino1 = declare_input("neutrino1")
local neutrino2 = declare_input("neutrino2")

parameters = {
    energy = 13000.,
    top_mass = 173.
}

inputs = {
    electron.reco_p4,
    bjet1.reco_p4,
    muon.reco_p4,
    bjet2.reco_p4,
    neutrino1.reco_p4,
    neutrino2.reco_p4
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = {electron.reco_p4, bjet1.reco_p4, muon.reco_p4, bjet2.reco_p4}
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
