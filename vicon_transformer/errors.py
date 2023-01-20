"""Custom exceptions of vicon_transformer."""


class ViconTransformerError(Exception):
    """Base error for custom errors of this package."""

    pass


class NotConnectedError(ViconTransformerError):
    pass


class ConnectionFailedError(ViconTransformerError):
    pass


class SubjectNotPresentError(ViconTransformerError):
    pass
