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
        'tf_p3::output',
        'tf_p4::output',
    }
else
    -- No transfer functions
    inputs_before_perm = {
        'input::particles/0',
        'input::particles/1',
        'input::particles/2',
        'input::particles/3',
    }
end

USE_PERM = true

if USE_PERM then
  -- Use permutator module to permutate input particles 0 and 2 using the MC
  inputs = {
    inputs_before_perm[1],
    'permutator::output/0',
    inputs_before_perm[3],
    'permutator::output/1',
  }
else
  -- No permutation, take particles as they come
  inputs = inputs_before_perm
end

configuration = {
    energy = 13000.,
    top_mass = 173.,
    top_width = 1.491500,
    W_mass = 80.419002,
    W_width = 2.047600,
}

vegas = {
    verbosity = 3
}

Flatter.flatter_s13 = { 
    -- CubaIndex() generates an input tag of type `cuba::ps_points/i`
    -- where `i` is automatically incremented each time the function is called.
    input = CubaIndex(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

Flatter.flatter_s134 = {
    input = CubaIndex(),
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

Flatter.flatter_s25 = {
    input = CubaIndex(), 
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

Flatter.flatter_s256 = {
    input = CubaIndex(),
    mass = parameter('top_mass'),
    width = parameter('top_width')
}

if USE_TF then
    GaussianTransferFunction.tf_p1 = {
        ps_point = CubaIndex(),
        reco_particle = 'input::particles/0',
        sigma = 0.05,
    }

    GaussianTransferFunction.tf_p2 = {
        ps_point = CubaIndex(),
        reco_particle = 'input::particles/1',
        sigma = 0.10,
    }

    -- Example for binned transfer function (only works on ingrid)
    -- BinnedTransferFunctionOnEnergy.tf_p2 = {
    --     ps_point = 'cuba::ps_points/5',
    --     reco_particle = 'input::particles/1',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }

    GaussianTransferFunction.tf_p3 = {
        ps_point = CubaIndex(),
        reco_particle = 'input::particles/2',
        sigma = 0.05,
    }

    GaussianTransferFunction.tf_p4 = {
        ps_point = CubaIndex(),
        reco_particle = 'input::particles/3',
        sigma = 0.10,
    }
    
    -- BinnedTransferFunctionOnEnergy.tf_p4 = {
    --     ps_point = 'cuba::ps_points/7',
    --     reco_particle = 'input::particles/3',
    --     file = '/home/fynu/swertz/tests_MEM/binnedTF/TF_generator/Control_plots_hh_TF.root',
    --     th2_name = 'Binned_Egen_DeltaE_Norm_jet',
    -- }
end

if USE_PERM then
    Permutator.permutator = {
        ps_point = CubaIndex(), 
        input = {
          inputs_before_perm[2],
          inputs_before_perm[4],
        }
    }
end

BlockD.blockd = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s134 = 'flatter_s134::s',
    s25 = 'flatter_s25::s',
    s256 = 'flatter_s256::s',
}

Boost.boost = {
    invisibles = {
        'blockd::invisibles',
    },

    particles = inputs
}


jacobians = {'flatter_s13::jacobian', 'flatter_s134::jacobian', 'flatter_s25::jacobian', 'flatter_s256::jacobian'}

if USE_TF then
    append(jacobians, {'tf_p1::TF_times_jacobian', 'tf_p2::TF_times_jacobian', 'tf_p3::TF_times_jacobian', 'tf_p4::TF_times_jacobian'})
end

MatrixElement.ttbar = {
  pdf = 'CT10nlo',

  matrix_element = 'pp_ttx_fully_leptonic',
  matrix_element_parameters = {
      card = '../MatrixElements/Cards/param_card.dat'
  },

  initialState = 'boost::output',

  invisibles = {
    input = 'blockd::invisibles',
    jacobians = 'blockd::jacobians',
    ids = {
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
    }
  },

  jacobians = jacobians
}
