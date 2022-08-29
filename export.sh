#!/bin/bash

set -e
VERSION=ki6.0.7_Debian

function handleDir {
    USER_ID=$(id -u)
    GROUP_ID=$(id -g)
    WORKDIR=.
    SUBDIR=$1

    echo "Handling ${SUBDIR}"

    cp kibot_generated.kibot.yaml $SUBDIR
    docker run --rm -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY \
           -v $(pwd)/$WORKDIR:/home/$USER/workdir \
           --user $USER_ID:$GROUP_ID \
           --env NO_AT_BRIDGE=1 \
           --workdir="/home/$USER" \
           --volume="/home/$USER/.local/share/kicad:/home/$USER/.local/share/kicad" \
           --volume="/home/$USER/.cache/kicad_v6.0:/home/$USER/.cache/kicad_v6.0" \
           --volume="/home/$USER/.cache/kicad:/home/$USER/.cache/kicad" \
           --volume="/home/$USER/.cache/kicost:/home/$USER/.cache/kicost" \
           --volume="/home/$USER/.config/kicad:/home/$USER/.config/kicad:rw" \
           --volume="/etc/group:/etc/group:ro" \
           --volume="/etc/passwd:/etc/passwd:ro" \
           --volume="/etc/shadow:/etc/shadow:ro" \
           setsoft/kicad_auto_test:${VERSION} /bin/bash -c "cd workdir/$SUBDIR; kibot"

    rm ${WORKDIR}/${SUBDIR}/kibot_generated.kibot.yaml
}

# Can't do this in parallel, since there's contention on files in ~/.config/kicad

handleDir QuadAttenuverter/ioboard
handleDir QuadAttenuverter/mainboard
handleDir QuadAttenuverter/faceplate

wait
