# SPDX-License-Identifier: BSD-3-Clause
"""Wrapper around Vicon Datastream SDK that simplifies setup and access to the data.

Most relevantly, the `ViconTransformer` class can provide poses of all subjects relative
to a configurable "origin subject".  This origin subject should be fixed in the scene
(e.g. screwed to a wall), such that it cannot move over time.  It then serves as a
clearly defined origin of the scene, that is independent of the actual origin used by
Vicon (which otherwise would need to be redefined after every re-calibration and is
generally difficult to set precisely).
"""
from __future__ import annotations

from .vicon_transformer_bindings import (
    BadResultError,
    NotConnectedError,
    SubjectData,
    SubjectNotVisibleError,
    UnknownSubjectError,
    ViconFrame,
    ViconReceiver as _ViconReceiver,
    ViconReceiverConfig,
    ViconTransformer,
    to_json,
    from_json,
)

__version__ = "1.0.0"


# extend ViconReceiver from bindings with enter/exit methods
class ViconReceiver(_ViconReceiver):
    """Wrapper around ViconReceiver implemented in C++."""

    def __enter__(self) -> ViconReceiver:
        """Connect to the Vicon server."""
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback) -> None:  # noqa[ANN001]
        """Disconnect from the vicon server."""
        self.disconnect()


__all__ = (
    "BadResultError",
    "NotConnectedError",
    "SubjectData",
    "SubjectNotVisibleError",
    "UnknownSubjectError",
    "ViconFrame",
    "ViconReceiver",
    "ViconReceiverConfig",
    "ViconTransformer",
    "to_json",
    "from_json",
)
