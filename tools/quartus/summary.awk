BEGIN {
    FS = ":"
    print ""
    print "Quartus resource usage:"
}

/^Total logic elements[[:space:]]*:/ || /^Total pins[[:space:]]*:/ || /^Total memory bits[[:space:]]*:/ {
    name = $1
    sub(/[[:space:]]+$/, "", name)
    if (split($2, values, "/") != 2) {
        print "Invalid resource value in fitter summary: " name > "/dev/stderr"
        failed = 1
        next
    }
    sub(/\(.*/, "", values[2])
    gsub(/[^0-9]/, "", values[1])
    gsub(/[^0-9]/, "", values[2])
    if (values[1] == "" || values[2] == "") {
        print "Invalid resource value in fitter summary: " name > "/dev/stderr"
        failed = 1
        next
    }
    used = values[1] + 0
    available = values[2] + 0
    printf "  %-20s used: %d / %d, free: %d\n", name, used, available, available - used
    found[name] = 1
}

END {
    if (!found["Total logic elements"] || !found["Total pins"] || !found["Total memory bits"]) {
        print "Fitter summary is missing one or more resource values" > "/dev/stderr"
        failed = 1
    }
    if (failed) exit 1
}
