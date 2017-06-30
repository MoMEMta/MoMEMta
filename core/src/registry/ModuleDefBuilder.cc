/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <momemta/ModuleDefBuilder.h>

#include <momemta/Logging.h>

#include <ModuleDefUtils.h>
#include <strings/Scanner.h>

namespace momemta {
namespace registration {

namespace {

// FIXME: Do proper error handling

class missing_attribute: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * Finalize an input or output definition given a \p spec
 *
 * \note Attributes must have already been finalized before calling this function
 *
 * \param spec The input or output spec
 * \param is_output True if finalizing an output, false otherwise
 * \param data Registration data of the module, containing the definition
 */
void finalizeInputOrOutput(StringPiece spec, bool is_output, ModuleRegistrationData& data) {
    using namespace strings;

    ArgDef def;
    StringPiece out;

    // An optional input starts with a '?'

    def.optional = spec.Consume("?");

    def.many = spec.Consume("*");

    if (! is_output) {
        // This can be a nested input declaration
        // Format is `<attr>/[<attr>/]*<name>`
        while (true) {
            if (! Scanner(spec)
                .RestartCapture()
                .ScanUntil('/')
                .StopCapture()
                .OneLiteral("/")
                .GetResult(&spec, &out)) {
                // No (more) match, exiting the loop
                break;
            }

            auto nested_attribute = out.ToString();

            // Ensure attribute exists
            auto attr = momemta::findAttr(nested_attribute, data.module_def);
            if (! attr) {
                std::string error = "Input definition for module " + data.module_def.name + " references a non-existing "
                            "attribute: " + nested_attribute;
                LOG(fatal) << error;
                throw missing_attribute(error);
            } else {
                def.nested_attributes.push_back(*attr);
            }
        }
    }

    // Parse <name>
    Scanner(spec)
            .One(Scanner::LETTER)
            .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
            .StopCapture()
            .AnySpace()
            .GetResult(&spec, &out);

    def.name = out.ToString();

    if (!is_output) {
        // Parse default value
        bool has_default = spec.Consume("=");
        if (has_default) {
            Scanner(spec)
                    .Any(Scanner::LETTER_DIGIT_UNDERSCORE_COLON)
                    .StopCapture()
                    .AnySpace()
                    .GetResult(&spec, &out);

            def.default_value = out.ToString();
            def.optional = true;
        }
    }

    if (is_output) {
        data.module_def.outputs.emplace_back(def);
    } else {
        data.module_def.inputs.emplace_back(def);
    }
}

void finalizeAttr(StringPiece spec, ModuleRegistrationData& data) {
    using namespace strings;

    AttrDef def;

    // A global attribute starts with a '^'.
    def.global = spec.Consume("^");

    // An optional attribute starts with '?'
    def.optional = spec.Consume("?");

    StringPiece out;

    // Parse `<name>:`
    Scanner(spec)
            .One(Scanner::LETTER)
            .Any(Scanner::LETTER_DIGIT_UNDERSCORE)
            .StopCapture()
            .AnySpace()
            .OneLiteral(":")
            .AnySpace()
            .GetResult(&spec, &out);

    def.name = out.ToString();

    // Read "<type>" or "list(<type>)".
    bool is_list = Scanner(spec)
            .OneLiteral("list")
            .AnySpace()
            .OneLiteral("(")
            .AnySpace()
            .GetResult(&spec);

    // Consume type
    Scanner(spec)
        .Any(Scanner::LOWERLETTER)
        .StopCapture()
        .AnySpace()
        .GetResult(&spec, &out);

    if (is_list) {
        def.type = "list(" + out.ToString() + ")";
    } else {
        def.type = out.ToString();
    }

    if (is_list) {
        spec.Consume(")");
    }

    // Check if there's a default value, and parse it
    if (spec.Consume("=")) {
        Scanner(spec)
            .Any(Scanner::LETTER_DIGIT_DASH_DOT_SLASH_UNDERSCORE)
            .StopCapture()
            .AnySpace()
            .GetResult(&spec, &out);
        def.default_value = out.ToString();
        def.optional = true;
    }

    data.module_def.attributes.emplace_back(def);
}

}

ModuleDefBuilder::ModuleDefBuilder(const std::string& name) {
    reg_data.module_def.name = name;

    if (name.length() > 1 && name[0] == '_')
        reg_data.module_def.internal = true;
}

ModuleDefBuilder& ModuleDefBuilder::Input(const std::string& spec) {
    inputs.emplace_back(spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::OptionalInput(const std::string& spec) {
    // Prefix spec with '?' to indicate an optional input
    inputs.emplace_back("?" + spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::Inputs(const std::string& spec) {
    // Prefix spec with '*' to indicate a list of inputs
    inputs.emplace_back("*" + spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::OptionalInputs(const std::string& spec) {
    // Prefix spec with '?*' to indicate optional inputs
    inputs.emplace_back("?*" + spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::Output(const std::string& spec) {
    outputs.emplace_back(spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::Attr(const std::string& spec) {
    attrs.emplace_back(spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::GlobalAttr(const std::string& spec) {
    // Prefix spec with '^' to indicate a global attribute
    attrs.emplace_back("^" + spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::OptionalAttr(const std::string& spec) {
    // Prefix spec with '?' to indicate an optional attribute
    attrs.emplace_back("?" + spec);
    return *this;
}

ModuleDefBuilder& ModuleDefBuilder::Sticky() {
    reg_data.module_def.sticky = true;
    return *this;
}

std::string ModuleDefBuilder::name() const {
    return reg_data.module_def.name;
}

ModuleRegistrationData ModuleDefBuilder::Build() const {

    auto data = reg_data;

    for (const auto& spec: attrs) {
        finalizeAttr(spec, data);
    }

    for (const auto& spec: inputs) {
        finalizeInputOrOutput(spec, false, data);
    }

    for (const auto& spec: outputs) {
        finalizeInputOrOutput(spec, true, data);
    }

    return data;
}

}
}