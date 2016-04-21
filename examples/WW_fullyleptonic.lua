function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end

M_W = 80.419002
W_W = 2.047600e+00

USE_TF = true

if USE_TF then
    -- With transfer functions
    inputs = {
        'tf_p1::output',
        'tf_p2::output',
    }
else
    -- No transfer functions
    inputs = {
        'input::particles/0',
        'input::particles/1',
    }
end

configuration = {
    energy = 13000.,
    W_mass = M_W
}

vegas = {
    verbosity = 3
}

Flatter.flatter_s13 = {
    input = "cuba::ps_points/0",
    mass = M_W,
    width = W_W
}



Flatter.flatter_s24 = {
    input = "cuba::ps_points/1",
    mass = M_W,
    width = W_W
}



if USE_TF then
    GaussianTransferFunction.tf_p1 = {
        ps_point = 'cuba::ps_points/2',
        reco_particle = 'input::particles/0',
        sigma = 0.10,
    }

    GaussianTransferFunction.tf_p2 = {
        ps_point = 'cuba::ps_points/3',
        reco_particle = 'input::particles/1',
        sigma = 0.10,
    }
    
end

BlockF.blockf = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    q1 = 'cuba::ps_points/4',
    q2 = 'cuba::ps_points/5'
}

Boost.boost = {
    invisibles = {
        'blockf::invisibles',
    },

    particles = inputs
}


jacobians = {'flatter_s13::jacobian', 'flatter_s24::jacobian'}

if USE_TF then
    append(jacobians, {'tf_p1::jacobian', 'tf_p2::jacobian'})
end

MatrixElement.WW = {
  pdf = 'CT10nlo',

  matrix_element = 'pp_ttx_fully_leptonic',
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
