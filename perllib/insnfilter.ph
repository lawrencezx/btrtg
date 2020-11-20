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
        $arch_8086 = "8086";
        $arch_186 = "186";
        $arch_286 = "286";
        $arch_386 = "386";
        $arch_486 = "486";
        $arch_pentium = "PENT";
        $arch_p6 = "P6";
        $feat_mmx = "MMX";
        $feat_sse = "SSE";
        $feat_sse2 = "SSE2";
        
        if ((grep /^$arch_8086/, @features) == 0 &&
            (grep /^$arch_186/, @features) == 0 &&
            (grep /^$arch_286/, @features) == 0 &&
            (grep /^$arch_386/, @features) == 0 &&
            (grep /^$arch_486/, @features) == 0 &&
            (grep /^$arch_pentium/, @features) == 0 &&
            (grep /^$arch_p6/, @features) == 0 &&
            (grep /^$feat_mmx/, @features) == 0 &&
            (grep /^$feat_sse/, @features) == 0 &&
            (grep /^$feat_sse2/, @features) == 0) {
            return 1;
        }

        $feat_3dnow = "3DNOW";
        $feat_cyrix = "CYRIX";
        if ((grep /^$feat_3dnow/, @features) != 0 ||
            (grep /^$feat_cyrix/, @features) != 0) {
            return 1;
        }
    }
    return 0;
}

1;
