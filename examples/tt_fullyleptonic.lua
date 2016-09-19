function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end

function copy_and_append(t1, t2)
    local t3 = {}

    append(t3, t1)
    append(t3, t2)

    return t3
end

load_modules('libempty_module.so')
load_modules('MatrixElements/dummy/libme_dummy.so')

USE_TF = true

if USE_TF then
    -- With transfer functions
    inputs_before_perm = {
        'tf_p1::output',
        'tf_p2::output',
        'tf_p3::output',
        'tf_p4::output',
    }
else
    -- No transfer functions
    inputs_before_perm = {
        'input::particles/1',
        'input::particles/2',
        'input::particles/3',
        'input::particles/4',
    }
end

USE_PERM = true

if USE_PERM then
  -- Use permutator module to permutate input particles 0 and 2 using the MC
  inputs = {
    inputs_before_perm[1],
    'permutator::output/1',
    inputs_before_perm[3],
    'permutator::output/2',
  }
else
  -- No permutation, take particles as they come
  inputs = inputs_before_perm
end

parameters = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600,
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
        reco_particle = 'input::particles/1',
        sigma = 0.05,
    }

    GaussianTransferFunctionOnEnergy.tf_p2 = {
        ps_point = add_dimension(),
        reco_particle = 'input::particles/2',
        sigma = 0.10,
    }

    -- Example for binned transfer function (only works on ingrid)
    -- BinnedTransferFunctionOnEnergy.tf_p2 = {
    --     ps_point = add_dimension(),
    --     reco_particle = 'input::particles/2',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }

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

    -- BinnedTransferFunctionOnEnergy.tf_p4 = {
    --     ps_point = add_dimension(),
    --     reco_particle = 'input::particles/4',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }
end

StandardPhaseSpace.phaseSpaceOut = {
    particles = inputs -- only on visible particles
}

-- Declare module before the permutator to test read-access in the pool
-- for non-existant values.
BlockD.blockd = {
    inputs = inputs,

    pT_is_met = true,

    s13 = 'flatter_s13::s',
    s134 = 'flatter_s134::s',
    s25 = 'flatter_s25::s',
    s256 = 'flatter_s256::s',
}

if USE_PERM then
    Permutator.permutator = {
        ps_point = add_dimension(),
        inputs = {
          inputs_before_perm[2],
          inputs_before_perm[4],
        }
    }
end

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

    DMEM.dmem = {
      x_start = 0.,
      x_end = 2000.,
      n_bins = 500,

      ps_weight = 'cuba::ps_weight',
      particles = full_inputs,
      me_output = 'ttbar::output',
    }

    DoubleSummer.integrand = {
        input = "ttbar::output"
    }

-- End of loop
integrand("integrand::sum")
