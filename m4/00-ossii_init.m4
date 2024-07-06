dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# define the name of the vendor
m4_define([ossii_VENDOR], ["Ministry of Digital Affairs"])

# define the product name
m4_define([ossii_PRODUCT_NAME], ["MODA ODF Web"])
# define the core office name
m4_define([ossii_CORE_OFFICE], ["modaodfsys"])

# define the name of the WSD (Web Socket Daemon)
m4_define([ossii_WSD_NAME], [modaodfweb])

# define the name of the library. This is the name of the shared object
m4_define([ossii_LIB_NAME], [MODAODFWEB])

AC_DEFUN([ossii_INIT],[
    AC_SUBST([PRODUCTNAME], [ossii_PRODUCT_NAME])
    AC_SUBST([CORE_OFFICE], [ossii_CORE_OFFICE])
    AC_SUBST([WSD_NAME], [ossii_WSD_NAME])
    AC_SUBST([OSSII_LIB_NAME], [ossii_LIB_NAME])
])


dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
