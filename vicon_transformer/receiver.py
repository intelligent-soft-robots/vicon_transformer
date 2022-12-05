from __future__ import annotations

import logging
import typing

import zmq

from .errors import NotConnectedError, ConnectionFailedError


class ZmqJsonReceiver:
    """Connect to a socket and receive JSON data via zmq.

    This class can be used in a ``with`` statement:

    .. code-block:: Python

        with ZmqJsonReceiver(...) as receiver:
            data = receiver.read()

    If used without ``with``, make sure to call :meth:`connect` and :meth:`close`:

    .. code-block:: Python

        receiver = ZmqJsonReceiver(...)
        receiver.connect()
        data = receiver.read()
        receiver.close()
    """

    def __init__(
        self,
        address: str = "tcp://10.42.2.29:5555",
        timeout_ms: int = 5000,
    ) -> None:
        """
        Args:
            address:  Address to which the ZMQ socket is connected.  Any protocol that
                is supported by ZMQ can be used.
            timeout_ms:  For how long to wait when no message is received.
                In milliseconds.
        """
        self._log = logging.getLogger(__name__)

        # type declarations
        self._socket: zmq.Socket
        self._context: zmq.Context

        self._is_connected = False
        self._address = address
        self._timeout_ms = timeout_ms

    def __del__(self) -> None:
        if self._is_connected:
            self.close()

    def __enter__(self) -> ZmqJsonReceiver:
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.close()

    def connect(self) -> None:
        """Connect to the address given to the constructor.

        Raises:
            ConnectionFailedError:  If connection cannot be established.
        """
        try:
            self._context = zmq.Context()
            self._socket = self._context.socket(zmq.SUB)
            self._socket.setsockopt(zmq.SUBSCRIBE, b"")
            self._socket.RCVTIMEO = self._timeout_ms

            self._log.info("Connect to %s", self._address)
            self._socket.connect(self._address)

            if self._socket.closed is True:
                raise ConnectionFailedError()

            self._is_connected = True
        except Exception as e:
            raise ConnectionFailedError(f"could not connect: {e}")

    def close(self) -> None:
        """Close connection.  Does nothing if not currently connected."""
        self._log.debug("disconnect: disconnecting...")
        if self._is_connected:
            self._context.destroy()
            if self._socket.closed is True:
                self._is_connected = False
                self._log.info("disconnected. Bye...")
            else:
                self._log.error("disconnecting failed!")
        else:
            self._log.info("already disconnected. Bye...")

    def read(self) -> typing.Any:
        """Read one json object from the socket.

        Returns:
            The received data.

        Raises:
            NotConnectedError:  If called without using a context manager or calling
                :meth:`connect()` first.
        """
        if not self._is_connected:
            raise NotConnectedError()

        return self._socket.recv_json()
