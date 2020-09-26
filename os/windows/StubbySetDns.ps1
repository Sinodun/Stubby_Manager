# Copyright 2020 Sinodun Internet Technologies Ltd.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at https://mozilla.org/MPL/2.0/.


Param([Switch] $Loopback = $false)

$ErrorActionPreference = "Stop"

Try {
    Get-NetAdapter -Physical | ForEach-Object {
        $ifname = $_.Name
        If ($Loopback) {
            set-dnsclientserveraddress $ifname -ServerAddresses ("127.0.0.1","0::1")
        } Else {
            set-dnsclientserveraddress $ifname -ResetServerAddresses
        }
    }
}
Catch {
}

exit $LASTEXITCODE
