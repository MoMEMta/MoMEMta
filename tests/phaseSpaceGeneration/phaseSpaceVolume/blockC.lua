local p2 = declare_input("p2")
local p3 = declare_input("p3")

load_modules('MatrixElements/dummy/libme_dummy.so')

parameters = {
    energy = 1000.,
    s12_mass = 100.,
    s12_width = 100.,
    s123_mass = 500.,
    s123_width = 500.,
}

cuba = {
    verbosity = 3,
    max_eval = 200000000,
    relative_accuracy = 0.0005,
    n_start = 20000000,
    seed = 5468960,
    ncores = 10,
    batch_size = 10000000
}

-- 'Flat' transfer functions to integrate over the visible particle's energies and angles
-- First |P|
FlatTransferFunctionOnP.tf_p_2 = {
    ps_point = add_dimension(),
    reco_particle = p2.reco_p4,
    min = 0.,
    max = parameters.energy/2,
}

-- Pass these outputs over for Phi
FlatTransferFunctionOnPhi.tf_phi_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_p_2::output',
}
FlatTransferFunctionOnPhi.tf_phi_3 = {
    ps_point = add_dimension(),
    reco_particle = p3.reco_p4,
}

-- Finally, do Theta
FlatTransferFunctionOnTheta.tf_theta_2 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_2::output',
}
FlatTransferFunctionOnTheta.tf_theta_3 = {
    ps_point = add_dimension(),
    reco_particle = 'tf_phi_3::output',
}

BreitWignerGenerator.flatter_s12 = {
    ps_point = add_dimension(),
    mass = parameter('s12_mass'),
    width = parameter('s12_width')
}

BreitWignerGenerator.flatter_s123 = {
    ps_point = add_dimension(),
    mass = parameter('s123_mass'),
    width = parameter('s123_width')
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = { 'tf_theta_2::output' },
}

BlockC.blockc = {
    p2 = 'tf_theta_2::output',
    p3 = 'tf_theta_3::output',
    s12 = 'flatter_s12::s',
    s123 = 'flatter_s123::s',
}

-- Loop

Looper.looper = {
    solutions = "blockc::solutions",
    path = Path("initial_state", "dummy", "integrand")
}

    gen_inputs = { 'tf_theta_2::output', 'looper::particles/1', 'looper::particles/2' }

    BuildInitialState.initial_state = {
        particles = gen_inputs
    }

    jacobians = {
      'tf_p_2::TF_times_jacobian',
      'tf_phi_2::TF_times_jacobian', 'tf_phi_3::TF_times_jacobian',
      'tf_theta_2::TF_times_jacobian', 'tf_theta_3::TF_times_jacobian',
      'flatter_s12::jacobian', 'flatter_s123::jacobian',
      'looper::jacobian', 'phaseSpaceOut::phase_space'
    }

    MatrixElement.dummy = {
      use_pdf = false,

      matrix_element = 'dummy_matrix_element',
      matrix_element_parameters = {},

      initialState = 'initial_state::partons',

      particles = {
        inputs = gen_inputs,
        ids = {
            { pdg_id = 1, me_index = 1 },
            { pdg_id = 1, me_index = 2 },
            { pdg_id = 1, me_index = 3 },
        }
      },

      jacobians = jacobians
    }

    DoubleLooperSummer.integrand = {
        input = "dummy::output"
    }

-- End of loop

integrand("integrand::sum")
