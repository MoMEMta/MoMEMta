-- Note: USE_PERM and USE_TF are defined in the C++ code and injected in lua before parsing this file

-- Register inputs
local electron = declare_input("electron")
local muon = declare_input("muon")
local bjet1 = declare_input("bjet1")
local bjet2 = declare_input("bjet2")

if USE_PERM then
    -- Automatically insert a Permutator module
    add_reco_permutations(bjet1, bjet2)
end

parameters = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600
}

cuba = {
    relative_accuracy = 0.01,
    verbosity = 3,
}

BreitWignerGenerator.flatter_s13 = {
    -- add_dimension() generates an input tag of type `cuba::ps_points/i`
    -- where `i` is automatically incremented each time the function is called.
    -- This function allows MoMEMta to track how many dimensions are needed for the integration.
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

if USE_TF then
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

    -- Example for binned transfer function (only works on ingrid)
    -- BinnedTransferFunctionOnEnergy.tf_p2 = {
    --     ps_point = add_dimension(),
    --     reco_particle = 'input::particles/2',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }

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

    -- BinnedTransferFunctionOnEnergy.tf_p4 = {
    --     ps_point = add_dimension(),
    --     reco_particle = 'input::particles/4',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }
end

-- If set_gen_p4 is not called, gen_p4 == reco_p4
inputs = {
    electron.gen_p4,
    bjet1.gen_p4,
    muon.gen_p4,
    bjet2.gen_p4
}

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs -- only on visible particles
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

-- Loop over block solutions

Looper.looper = {
    solutions = "blockd::solutions",
    path = Path("boost", "ttbar", "dmem", "integrand")
}

    -- Block D produce solutions with two particles
    full_inputs = copy_and_append(inputs, {'looper::particles/1', 'looper::particles/2'})

    BuildInitialState.boost = {
        do_transverse_boost = true,
        particles = full_inputs
    }

    jacobians = {'flatter_s13::jacobian', 'flatter_s134::jacobian', 'flatter_s25::jacobian', 'flatter_s256::jacobian'}

    if USE_TF then
        append(jacobians, {'tf_p1::TF_times_jacobian', 'tf_p2::TF_times_jacobian', 'tf_p3::TF_times_jacobian', 'tf_p4::TF_times_jacobian'})
    end

    append(jacobians, {'phaseSpaceOut::phase_space', 'looper::jacobian'})

    MatrixElement.ttbar = {
      pdf = 'CT10nlo',
      pdf_scale = parameter('top_mass'),

      matrix_element = 'pp_ttx_fully_leptonic',
      matrix_element_parameters = {
          card = '../MatrixElements/Cards/param_card.dat',
      },

      override_parameters = {
          mdl_MT = parameter('top_mass'),
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

    DMEM.dmem = {
      x_start = 0.,
      x_end = 2000.,
      n_bins = 500,

      ps_weight = 'cuba::ps_weight',
      particles = full_inputs,
      me_output = 'ttbar::output',
    }

    DoubleLooperSummer.integrand = {
        input = "ttbar::output"
    }

-- End of loop
integrand("integrand::sum")
