# -*- perl -*-
#
# Filter all the unsupported instructions by CPU and feature.
#

#
# insnfilter(line)
#
sub insnfilter($) {
    my($line) = @_;
    if ($line =~ /^\s*(\S+)\s+(\S+)\s+(\S+|\[.*\])\s+(\S+)\s*$/) {
        @features = split(/,/,$4);
        $cpu_8086 = "8086";
        $cpu_186 = "186";
        $cpu_286 = "286";
        $cpu_386 = "386";
        $cpu_486 = "486";
        $cpu_pentium = "PENT";
        $cpu_p6 = "P6";
        $cpu_katmai = "KATMAI";
        $cpu_willamette = "WILLAMETTE";
        
        if ((grep /^$cpu_8086/, @features) == 0 &&
            (grep /^$cpu_186/, @features) == 0 &&
            (grep /^$cpu_286/, @features) == 0 &&
            (grep /^$cpu_386/, @features) == 0 &&
            (grep /^$cpu_486/, @features) == 0 &&
            (grep /^$cpu_pentium/, @features) == 0 &&
            (grep /^$cpu_p6/, @features) == 0 &&
            (grep /^$cpu_katmai/, @features) == 0 &&
            (grep /^$cpu_willamette/, @features) == 0) {
            return 1;
        }

        $extension_3dnow = "3DNOW";
        $extension_cyrix = "CYRIX";
        if ((grep /^$extension_3dnow/, @features) != 0 ||
            (grep /^$extension_cyrix/, @features) != 0) {
            return 1;
        }
    }
    return 0;
}

1;
