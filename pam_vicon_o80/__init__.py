"""Interface for the PAM Vicon system."""
from .pam_vicon import PamVicon, NoFrameDataError


__all__ = ("PamVicon", "NoFrameDataError")
