function append(t1, t2)
    for i = 1, #t2 do
        t1[#t1 + 1] = t2[i]
    end

    return t1
end


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

    inputs = inputs_before_perm


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
    -- add_dimension() generates an input tag of type `cuba::ps_points/i`
    -- where `i` is automatically incremented each time the function is called.
    -- This function allows MoMEMta to track how many dimensions are needed for the integration.
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}

BreitWignerGenerator.flatter_s24 = {
    ps_point = add_dimension(),
    mass = parameter('W_mass'),
    width = parameter('W_width')
}


if USE_TF then
    GaussianTransferFunction.tf_p1 = {
        ps_point = add_dimension(),
        reco_particle = 'input::particles/1',
        sigma = 0.05,
    }

    GaussianTransferFunction.tf_p2 = {
        ps_point = add_dimension(),
        reco_particle = 'input::particles/2',
        sigma = 0.10,
    }
end

BlockF.blockf = {
    inputs = inputs,

    s13 = 'flatter_s13::s',
    s24 = 'flatter_s24::s',
    q1 = add_dimension(),
    q2 = add_dimension()
}

Looper.looper = {
    solutions = "blockf::solutions",
    path = Path("initial_state", "WW", "integrand")
}

-- Loop

    BuildInitialState.initial_state = {
        solution = 'looper::solution',
        particles = inputs
    }

    jacobians = {'flatter_s13::jacobian', 'flatter_s24::jacobian'}

    if USE_TF then
        append(jacobians, {'tf_p1::TF_times_jacobian', 'tf_p2::TF_times_jacobian'})
    end

    MatrixElement.WW = {
      pdf = 'CT10nlo',
      pdf_scale = parameter('W_mass'),

      matrix_element = 'pp_WW_fully_leptonic_sm_P1_Sigma_sm_uux_epvemumvmx',
      matrix_element_parameters = {
          card = '../MatrixElements/Cards/param_card.dat'
      },

      initialState = 'initial_state::partons',

      invisibles = {
        input = 'looper::solution',
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

    DoubleSummer.integrand = {
        input = "WW::output"
    }

-- End of loop

integrand("integrand::sum")
