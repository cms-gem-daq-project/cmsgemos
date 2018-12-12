#! /usr/bin/env python2

import xml.etree.ElementTree as ET

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

    def subElement(self, parent):
        """
        Creates subElements of parent defining this field in XSD syntax
        """
        if len(self.childNames) == 0:
            ET.SubElement(parent, 'xs:element', {
                'name': self.name,
                'type': 'xs:unsignedInt'
            })
        else:
            # FIELD_NAME element
            element = ET.SubElement(parent, 'xs:element', {
                'name': '{}_NAME'.format(self.name),
            })
            simpleT = ET.SubElement(element, 'xs:simpleType')
            restriction = ET.SubElement(simpleT, 'xs:restriction', {
                'base': 'xs:string'
            })
            for childName in self.childNames:
                ET.SubElement(restriction, 'xs:enumeration', {
                    'value': childName
                })
            # FIELD_VALUE element
            ET.SubElement(parent, 'xs:element', {
                'name': '{}_VALUE'.format(self.name),
                'type': 'xs:unsignedInt'
            })

    def cppName(self, capitalizeFirstLetter = False):
        """
        Returns the name used to represent this C++ code
        """
        return _toCamelCase(self.name, capitalizeFirstLetter)

    def __repr__(self):
        return 'Field:{}'.format(self.name)

def _makeXml(config):
    extTableName = _checkedJsonGet(config, 'extension table name', unicode)
    kindOfPart = _checkedJsonGet(config, 'kind of part', unicode)
    fields = [ Field(f) for f in _checkedJsonGet(config, 'fields', list) ]

    schema = ET.Element('xs:schema', {
        'xmlns:xs': 'http://www.w3.org/2001/XMLSchema'
    })

    # Include common definitions
    redef = ET.SubElement(schema, 'xs:redefine', {'schemaLocation': 'common.xsd'})

    # Redefine the ExtensionTableName type to restrict its contents to extTableName
    simpleT = ET.SubElement(redef, 'xs:simpleType', {'name': 'ExtensionTableName'})
    restrict = ET.SubElement(simpleT, 'xs:restriction', {'base': 'ExtensionTableName'})
    ET.SubElement(restrict, 'xs:enumeration', {'value': extTableName})

    # Redefine the KindOfPart type to restrict its contents to kindOfPart
    simpleT = ET.SubElement(redef, 'xs:simpleType', {'name': 'KindOfPart'})
    restrict = ET.SubElement(simpleT, 'xs:restriction', {'base': 'KindOfPart'})
    ET.SubElement(restrict, 'xs:enumeration', {'value': kindOfPart})

    # Extend the Data type with elements for all fields
    cplxType = ET.SubElement(redef, 'xs:complexType', {'name': 'Data'})
    cplxContent = ET.SubElement(cplxType, 'xs:complexContent')
    extension = ET.SubElement(cplxContent, 'xs:extension', {'base': 'Data'})
    sequence = ET.SubElement(extension, 'xs:all')
    for f in fields:
        f.subElement(sequence)

    return ET.ElementTree(schema)

def _makeHeader(config, className):
    template = '''/*
 * THIS FILE WAS GENERATED
 *
 * Changes will be overwritten. Modify parseDef.py instead.
 */

#ifndef GEM_ONLINEDB_DETAIL_{baseNameCaps}GEN_H
#define GEM_ONLINEDB_DETAIL_{baseNameCaps}GEN_H

#include <cstdint>

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
        privateMembers += '''
                std::int32_t {};'''.format(f.cppName())

    publicMembers = ''
    for f in fields:
        publicMembers += '''
                std::int32_t get{1}() const {{ return {0}; }}
                void set{1}(std::int32_t value) {{ {0} = value; }}
                '''.format(f.cppName(False), f.cppName(True))

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
                           partReference = partReference)

def _makeCpp(config, className):
    template = '''/*
 * THIS FILE WAS GENERATED
 *
 * Changes will be overwritten. Modify parseDef.py instead.
 */

#include "gem/onlinedb/detail/{baseName}Gen.h"

namespace gem {{
    namespace onlinedb {{
        namespace detail {{
            bool {baseName}Gen::operator== (const {baseName}Gen &other) const
            {{
                return true
{operatorEq};
            }}

            RegisterData {baseName}Gen::getRegisterData() const
            {{
                RegisterData data;
{getRegisterData}

                return data;
            }}

            void {baseName}Gen::readRegisterData(const RegisterData &data)
            {{
{readRegisterData}
            }}
        }} /* namespace detail */
    }} /* namespace onlinedb */
}} /* namespace gem */
'''
    fields = [ Field(f) for f in _checkedJsonGet(config, 'fields', list) ]

    getRegisterData = ''
    for f in fields:
        getRegisterData += '''
                data["{0}"] = get{1}();'''.format(f.name, f.cppName(True))

    readRegisterData = ''
    for f in fields:
        readRegisterData += '''
                set{1}(data.at("{0}"));'''.format(f.name, f.cppName(True))

    operatorEq = ''
    for f in fields:
        operatorEq += '''
                    && {0} == other.{0}'''.format(f.cppName())

    return template.format(baseName = className,
                           getRegisterData = getRegisterData,
                           readRegisterData = readRegisterData,
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

    print('-- Generating: schema/{}.xsd'.format(baseFileName))
    _makeXml(config).write('schema/{}.xsd'.format(baseFileName), encoding='UTF-8')

    headerFileName = 'include/gem/onlinedb/detail/{}Gen.h'.format(baseFileName)
    with open(headerFileName, 'w') as f:
        print('-- Generating: {}'.format(f.name))
        f.write(_makeHeader(config, baseFileName))

    headerFileName = 'src/common/{}Gen.cc'.format(baseFileName)
    with open(headerFileName, 'w') as f:
        print('-- Generating: {}'.format(f.name))
        f.write(_makeCpp(config, baseFileName))
