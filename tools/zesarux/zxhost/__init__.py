"""Python protocol codec and host-side session for ZXHost targets."""

from .protocol import ZXHostProtocol
from .basic import ZXHostBasic
from .manifest import (
    BlockRecord,
    LaunchConfig,
    ProgramManifest,
    ZXHostManifestBuilder,
)

__all__ = [
    "ZXHostProtocol",
    "ZXHostSession",
    "ZXHostBasic",
    "BlockRecord",
    "LaunchConfig",
    "ProgramManifest",
    "ZXHostManifestBuilder",
]


def __getattr__(name):
    if name == "ZXHostSession":
        from .session import ZXHostSession
        return ZXHostSession
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
