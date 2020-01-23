#! /usr/bin/env python2

import itertools as it

def _checkedJsonGet(json, fieldName, targetType):
    """
    Returns a value from a json dictionary only if it has the right type
    """
    assert(fieldName in json)
    assert(type(json[fieldName]) == targetType)
    return json[fieldName]

def _toCamelCase(snake_string, capitalizeFirstLetter=False):
    """
    Converts a name to camelCase.
    """
    # Adapted from https://stackoverflow.com/a/19053800
    components = snake_string.split('_')
    first = components[0]
    if capitalizeFirstLetter:
        first = first.title()
    else:
        first = first.lower()
    # We capitalize the first letter of each component except the first one
    # with the 'title' method and join them together.
    return first + ''.join(x.title() for x in components[1:])

class Field(object):
    """
    Represents a field in the database.
    """
    def __init__(self, json):
        self.name = _checkedJsonGet(json, 'name', unicode)

        if 'child names' in json:
            self.childNames = _checkedJsonGet(json, 'child names', list)
            for child in self.childNames:
                assert(type(child) == unicode)
        else:
            self.childNames = []

        if 'count' in json:
            self.count = _checkedJsonGet(json, 'count', list)
        else:
            self.count = None

        if 'cpp name' in json:
            self._cppName = _checkedJsonGet(json, 'cpp name', unicode)
        else:
            self._cppName = _toCamelCase(self.name)

        if 'cpp type' in json:
            self._cppType = _checkedJsonGet(json, 'cpp type', unicode)
        else:
            self._cppType = 'std::uint32_t'

    def cppName(self, capitalizeFirstLetter = False):
        """
        Returns the name used to represent this C++ code
        """
        if capitalizeFirstLetter:
            return self._cppName[0].upper() + self._cppName[1:]
        else:
            return self._cppName

    def cppType(self):
        """
        Returns the C++ type to use for this field
        """
        if self.count is None:
            return self._cppType

        code = '{}'
        for n in self.count:
            code = code.format('std::array<{{}}, {}>'.format(n))
        return code.format(self._cppType)

    def cppField(self):
        """
        Returns a declaration for this field in C++
        """
        if self.count is None:
            return '''
                {} m_{};'''.format(self.cppType(), self.cppName())
        else:
            # Plural form
            return '''
                {} m_{}s;'''.format(self.cppType(), self.cppName())

    def cppGetters(self):
        """
        Returns the declaration of C++ get() functions for this field
        """
        if self.count is None:
            return '''
                {2} get{1}() const {{ return m_{0}; }};'''.format(self.cppName(False),
                                                                  self.cppName(True),
                                                                  self.cppType())
        else:
            code = '''
                const {2} &get{1}s() const {{ return m_{0}s; }};
                {2} &get{1}s() {{ return m_{0}s; }};'''.format(
                    self.cppName(False),
                    self.cppName(True),
                    self.cppType())
            code += '''
                {} get{}({}) const {{ return m_{}s{}; }};'''.format(
                    self._cppType,
                    self.cppName(True),
                    self.indices('decl'),
                    self.cppName(False),
                    self.indices('at'))
            return code

    def cppSetters(self):
        """
        Returns the declaration of C++ set() functions for this field
        """
        if self.count is None:
            return '''
                void set{1}({2} value) {{ m_{0} = value; }};'''.format(
                    self.cppName(False),
                    self.cppName(True),
                    self._cppType)
        else:
            code = '''
                void set{}s(const {} &value) {{ m_{}s = value; }};'''.format(
                    self.cppName(True),
                    self.cppType(),
                    self.cppName(False))
            code += '''
                void set{}({}, {} value) {{ m_{}s{} = value; }};'''.format(
                    self.cppName(True),
                    self.indices('decl'),
                    self._cppType,
                    self.cppName(False),
                    self.indices('at'))
            return code

    def cppEq(self):
        """
        Returns the C++ code to add for this field in operator==
        """
        if self.count is None:
            return '''
                    && get{0}() == other.get{0}()'''.format(self.cppName(True))
        else:
            return '''
                    && get{0}s() == other.get{0}s()'''.format(self.cppName(True))

    def ranges(self):
        """
        Returns the list of ranges in which a multidimensional variable iterates
        """
        if self.count is None:
            return []
        else:
            return [range(n) for n in self.count]

    def indices(self, mode, arg=None):
        """
        Returns the list of indices used for this field
        """
        names = ['i', 'j', 'k']
        if mode == 'at':
            return ''.join('.at({})'.format(n) for n in names[:len(self.count)])
        if mode == 'use':
            return ', '.join(['{}'.format(a) for a in arg])
        elif mode == 'decl':
            return ', '.join(['std::size_t ' + n for n in names][:len(self.count)])
        else:
            assert(False)

    def cppReadRegisterData(self):
        """
        Returns the C++ code to add for this field in readRegisterData()
        """
        if self._cppType != 'std::uint32_t':
            return '''
                // {}: type not supported'''.format(self.name)
        if self.count is None:
            return '''
                set{1}(data.at("{0}"));'''.format(self.name, self.cppName(True))
        else:
            code = ''
            for ijk in it.product(*self.ranges()):
                code += '''
                set{1}({2}, data.at("{0}"));'''.format(self.name.format(*ijk),
                                                       self.cppName(True),
                                                       self.indices('use', ijk))
            return code

    def cppGetRegisterData(self):
        """
        Returns the C++ code to add for this field in getRegisterData()
        """
        if self._cppType != 'std::uint32_t':
            return '''
                // {}: type not supported'''.format(self.name)
        if self.count is None:
            return '''
                data["{0}"] = get{1}();'''.format(self.name, self.cppName(True))
        else:
            code = ''
            print self.count
            for ijk in it.product(*self.ranges()):
                code += '''
                data["{0}"] = get{1}({2});'''.format(self.name.format(*ijk),
                                                     self.cppName(True),
                                                     self.indices('use', ijk))
            return code

    def cppFromJSONImpl(self):
        """
        Returns the C++ code to add for this field in from_json()
        """
        if self.count is None:
            return '''
                data.set{1}(json.at("{0}"));'''.format(self.name, self.cppName(True))
        else:
            code = ''
            for ijk in it.product(*self.ranges()):
                code += '''
                data.set{1}({2}, json.at("{0}"));'''.format(self.name.format(*ijk),
                                                            self.cppName(True),
                                                            self.indices('use', ijk))
            return code

    def cppToJSONImpl(self):
        """
        Returns the C++ code to add for this field in to_json()
        """
        if self.count is None:
            return '''
                json["{0}"] = data.get{1}();'''.format(self.name, self.cppName(True))
        else:
            code = ''
            for ijk in it.product(*self.ranges()):
                code += '''
                json["{0}"] = data.get{1}({2});'''.format(self.name.format(*ijk),
                                                          self.cppName(True),
                                                          self.indices('use', ijk))
            return code

    def __repr__(self):
        return 'Field:{}'.format(self.name)

def _makeHeader(config, className):
    template = '''/*
 * THIS FILE WAS GENERATED
 *
 * Changes will be overwritten. Modify parseDef.py instead.
 */

#ifndef GEM_ONLINEDB_DETAIL_{baseNameCaps}GEN_H
#define GEM_ONLINEDB_DETAIL_{baseNameCaps}GEN_H

#include <cstdint>

#include <nlohmann/json.hpp>

{extraIncludesCode}
#include "gem/onlinedb/ConfigurationTraits.h"
#include "gem/onlinedb/PartReference.h"
#include "gem/onlinedb/detail/RegisterData.h"

namespace gem {{
    namespace onlinedb {{
        namespace detail {{
            class {baseName}Gen
            {{
            private:{privateMembers}

            protected:
                explicit {baseName}Gen() {{ }}

            public:
                RegisterData getRegisterData() const;
                void readRegisterData(const RegisterData &data);

                bool operator== (const {baseName}Gen &other) const;
{publicMembers}
            }};

            void to_json(nlohmann::json &json, const {baseName}Gen &data);
            void from_json(const nlohmann::json &json, {baseName}Gen &data);
        }} /* namespace detail */

        template<>
        class ConfigurationTraits<detail::{baseName}Gen>
        {{
        public:
            static std::string extTableName() {{ return "{extTableName}"; }}
            static std::string typeName() {{ return "{typeName}"; }}
            static std::string kindOfPart() {{ return "{kindOfPart}"; }}
            using PartType = {partReference};
        }};
    }} /* namespace onlinedb */
}} /* namespace gem */

#endif // GEM_ONLINEDB_DETAIL_{baseNameCaps}GEN_H
'''
    fields = [ Field(f) for f in _checkedJsonGet(config, 'fields', list) ]

    privateMembers = ''
    for f in fields:
        privateMembers += f.cppField()

    publicMembers = ''
    for f in fields:
        publicMembers += f.cppGetters()
        publicMembers += "\n"
        publicMembers += f.cppSetters()

    extraIncludesCode = ''
    if 'extra includes' in config:
        extraIncludes = _checkedJsonGet(config, 'extra includes', list)
        for inc in extraIncludes:
            extraIncludesCode += '#include {}\n'.format(inc);

    extTableName = _checkedJsonGet(config, 'extension table name', unicode)
    typeName = _checkedJsonGet(config, 'type name', unicode)
    kindOfPart = _checkedJsonGet(config, 'kind of part', unicode)
    partReference = _checkedJsonGet(config, 'part reference', unicode)

    return template.format(baseName = className,
                           baseNameCaps = className.upper(),
                           privateMembers = privateMembers,
                           publicMembers = publicMembers,
                           extTableName = extTableName,
                           typeName = typeName,
                           kindOfPart = kindOfPart,
                           partReference = partReference,
                           extraIncludesCode = extraIncludesCode)

def _makeCpp(config, className):
    template = '''/*
 * THIS FILE WAS GENERATED
 *
 * Changes will be overwritten. Modify parseDef.py instead.
 */

#include <nlohmann/json.hpp>

#include "gem/onlinedb/detail/{baseName}Gen.h"

namespace gem {{
    namespace onlinedb {{
        namespace detail {{
            bool {baseName}Gen::operator== (const {baseName}Gen &other) const
            {{
                return true{operatorEq};
            }}

            RegisterData {baseName}Gen::getRegisterData() const
            {{
                RegisterData data;
{getRegisterData}

                return data;
            }}

            void {baseName}Gen::readRegisterData(const RegisterData &data)
            {{{readRegisterData}
            }}

            void from_json(const nlohmann::json &json, {baseName}Gen &data)
            {{{fromJSON}
            }}

            void to_json(nlohmann::json &json, const {baseName}Gen &data)
            {{{toJSON}
            }}
        }} /* namespace detail */
    }} /* namespace onlinedb */
}} /* namespace gem */
'''
    fields = [ Field(f) for f in _checkedJsonGet(config, 'fields', list) ]

    getRegisterData = ''
    for f in fields:
        getRegisterData += f.cppGetRegisterData()

    readRegisterData = ''
    for f in fields:
        readRegisterData += f.cppReadRegisterData()

    fromJSON = ''
    for f in fields:
        fromJSON += f.cppFromJSONImpl()

    toJSON = ''
    for f in fields:
        toJSON += f.cppToJSONImpl()

    operatorEq = ''
    for f in fields:
        operatorEq += f.cppEq()

    return template.format(baseName = className,
                           getRegisterData = getRegisterData,
                           readRegisterData = readRegisterData,
                           fromJSON = fromJSON,
                           toJSON = toJSON,
                           operatorEq = operatorEq)

if __name__ == '__main__':
    import argparse
    import json
    import os

    parser = argparse.ArgumentParser(
        description='Creates C++ and XSD files from a JSON file')
    parser.add_argument('inputFile', metavar='input.json', type=file,
                        help='The input JSON file')
    args = parser.parse_args()

    config = json.load(args.inputFile)

    # Get the file name without extension nor base path (ie bar in /foo/bar.ext)
    baseFileName = os.path.splitext(os.path.basename(args.inputFile.name))[0]

    headerFileName = 'include/gem/onlinedb/detail/{}Gen.h'.format(baseFileName)
    with open(headerFileName, 'w') as f:
        print('-- Generating: {}'.format(f.name))
        f.write(_makeHeader(config, baseFileName))

    headerFileName = 'src/common/{}Gen.cc'.format(baseFileName)
    with open(headerFileName, 'w') as f:
        print('-- Generating: {}'.format(f.name))
        f.write(_makeCpp(config, baseFileName))
