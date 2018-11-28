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
    ET.SubElement(schema, 'xs:include', {'schemaLocation': 'common.xsd'})

    # Redefine the ExtensionTableName type to restrict its contents to extTableName
    redef = ET.SubElement(schema, 'xs:redefine', {'schemaLocation': 'common.xsd'})
    simpleT = ET.SubElement(redef, 'xs:simpleType', {'name': 'ExtensionTableName'})
    restrict = ET.SubElement(simpleT, 'xs:restriction', {'base': 'ExtensionTableName'})
    ET.SubElement(restrict, 'xs:enumeration', {'value': extTableName})

    # Redefine the KindOfPart type to restrict its contents to kindOfPart
    redef = ET.SubElement(schema, 'xs:redefine', {'schemaLocation': 'common.xsd'})
    simpleT = ET.SubElement(redef, 'xs:simpleType', {'name': 'KindOfPart'})
    restrict = ET.SubElement(simpleT, 'xs:restriction', {'base': 'KindOfPart'})
    ET.SubElement(restrict, 'xs:enumeration', {'value': kindOfPart})

    # Extend the Data type with elements for all fields
    redef = ET.SubElement(schema, 'xs:redefine', {'schemaLocation': 'common.xsd'})
    cplxType = ET.SubElement(redef, 'xs:complexType', {'name': 'Data'})
    cplxContent = ET.SubElement(cplxType, 'xs:complexContent')
    extension = ET.SubElement(cplxContent, 'xs:extension', {'base': 'Data'})
    sequence = ET.SubElement(extension, 'xs:sequence')
    for f in fields:
        f.subElement(sequence)

    return ET.ElementTree(schema)

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

    _makeXml(config).write('schema/{}.xsd'.format(baseFileName), encoding='UTF-8')
