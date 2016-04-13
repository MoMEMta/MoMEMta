#! /bin/bash

# These two variables are not set when building a PR from a fork, for security reasons.
# Just ignore if it's the case
if [[ -z "$encrypted_31d8eebdbc2d_key" || -z "$encrypted_31d8eebdbc2d_iv" ]]; then
    exit 0;
fi

openssl aes-256-cbc -K $encrypted_31d8eebdbc2d_key -iv $encrypted_31d8eebdbc2d_iv -in travis/travisci_rsa.enc -out travis/travisci_rsa -d

chmod 0600 travis/travisci_rsa
cp travis/travisci_rsa ~/.ssh/id_rsa
