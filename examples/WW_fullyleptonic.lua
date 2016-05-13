function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end

load_modules('libempty_module.so')
load_modules('MatrixElements/dummy/libme_dummy.so')

USE_TF = true

if USE_TF then
    -- With transfer functions
    inputs_before_perm = {
        'tf_p1::output',
        'tf_p2::output',
    }
else
    -- No transfer functions
    inputs_before_perm = {
        'input::particles/1',
        'input::particles/2',
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
    W_mass = 80.419002,
    W_width = 2.047600,
}

cuba = {
    relative_accuracy = 0.01,
    verbosity = 3
}

BreitWignerGenerator.flatter_s13 = {
    -- getpspoint() generates an input tag of type `cuba::ps_points/i`
    -- where `i` is automatically incremented each time the function is called.
    ps_point = getpspoint(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BreitWignerGenerator.flatter_s24 = {
    ps_point = getpspoint(),
    mass = parameter('top_mass'),
    width = parameter('top_width')
}


if USE_TF then
    GaussianTransferFunction.tf_p1 = {
        ps_point = getpspoint(),
        reco_particle = 'input::particles/1',
        sigma = 0.05,
    }

    GaussianTransferFunction.tf_p2 = {
        ps_point = getpspoint(),
        reco_particle = 'input::particles/2',
        sigma = 0.10,
    }

    -- Example for binned transfer function (only works on ingrid)
    -- BinnedTransferFunctionOnEnergy.tf_p2 = {
    --     ps_point = getpspoint(),
    --     reco_particle = 'input::particles/2',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }


end

if USE_PERM then
    Permutator.permutator = {
        ps_point = getpspoint(),
        inputs = {
          inputs_before_perm[2],
          inputs_before_perm[4],
        }
    }
end

BlockF.blockf = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    q1 = getpspoint(),
    q2 = getpspoint()
}

Boost.boost = {
    invisibles = {
        'blockf::invisibles',
    },

    particles = inputs
}


jacobians = {'flatter_s13::jacobian', 'flatter_s24::jacobian'}

if USE_TF then
    append(jacobians, {'tf_p1::TF_times_jacobian', 'tf_p2::TF_times_jacobian'})
end

MatrixElement.WW = {
  pdf = 'CT10nlo',

  matrix_element = 'example_WW_llvv',
  matrix_element_parameters = {
      card = '../MatrixElements/Cards/param_card.dat'
  },

  initialState = 'boost::output',

  invisibles = {
    input = 'blockf::invisibles',
    jacobians = 'blockf::jacobians',
    ids = {
      {
        pdg_id = 12,
        me_index = 2,
      },

      {
        pdg_id = -14,
        me_index = 4,
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
        pdg_id = 13,
        me_index = 3,
      },
    }
  },

  jacobians = jacobians
}

DMEM.dmem_WW    = {
  x_start = 0.,
  x_end = 2000.,
  n_bins = 500,
  ps_weight = 'cuba::ps_weight',
  particles = inputs,
  invisibles = 'blockf::invisibles',
  integrands = 'WW::integrands',
}