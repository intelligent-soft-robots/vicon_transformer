# SPDX-License-Identifier: BSD-3-Clause
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
    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
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
