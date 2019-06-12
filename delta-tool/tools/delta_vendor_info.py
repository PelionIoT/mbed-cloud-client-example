#----------------------------------------------------------------------------
#   The confidential and proprietary information contained in this file may
#   only be used by a person authorised under and to the extent permitted
#   by a subsisting licensing agreement from ARM Limited or its affiliates.
#
#          (C) COPYRIGHT 2019 ARM Limited or its affiliates.
#              ALL RIGHTS RESERVED
#
#   This entire notice must be reproduced on all copies of this file
#   and copies of this file may only be made by a person if such person is
#   permitted to do so under the terms of a subsisting license agreement
#   from ARM Limited or its affiliates.
#----------------------------------------------------------------------------
# -*- coding: utf-8 -*-
#
# This file has been generated using asn1ate (v <unknown>) from 'delta-vendor-info.asn1'
# Last Modified on 2019-02-28 09:57:38.442092
from pyasn1.type import univ, char, namedtype, namedval, tag, constraint, useful


class DeltaInfo(univ.Sequence):
    pass


DeltaInfo.componentType = namedtype.NamedTypes(
    namedtype.NamedType('deltaVariant', univ.Enumerated(namedValues=namedval.NamedValues(('reserved', 0), ('arm-stream-diff-lz4', 1)))),
    namedtype.NamedType('precursorDigest', univ.OctetString()),
    namedtype.NamedType('deltaDigest', univ.OctetString()),
    namedtype.NamedType('deltaSize', univ.Integer())
)
