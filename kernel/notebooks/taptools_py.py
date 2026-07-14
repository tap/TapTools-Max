"""ctypes bridge to the TapTools C ABI, shared by the verification notebooks.

Loads build_capi/libtaptools_capi.{so,dylib,dll} relative to the kernel root
(kernel/ in the TapTools repo), building it first if missing (requires cmake
in PATH):

    cmake -B kernel/build_capi -S kernel/tools/capi
    cmake --build kernel/build_capi

The C ABI (kernel/tools/capi/) wraps the *same* portable DSP headers the Max
externals compile — so the notebooks exercise the real shipping code, not a
Python re-implementation. Currently exposes tap.convolve~'s
taptools::conv_engine (uniformly-partitioned overlap-save convolution) through
the `Convolver` class.

Copyright 2003-2026 Timothy Place. New BSD License.
"""

from __future__ import annotations

import ctypes
import pathlib
import subprocess
import sys

import numpy as np

# The kernel root (this file lives in kernel/notebooks/).
KERNEL = pathlib.Path(__file__).resolve().parent.parent

# Categorical palette for the notebooks (colorblind-safe, fixed assignment
# order — never cycled). Sequential maps use viridis; diverging use RdBu_r.
PALETTE = ["#4269d0", "#efb118", "#ff725c", "#6cc5b0", "#3ca951", "#ff8ab7", "#a463f2"]

_BUILD = KERNEL / "build_capi"


def _lib_path() -> pathlib.Path:
    stem = "taptools_capi"
    names = {"linux": f"lib{stem}.so", "darwin": f"lib{stem}.dylib", "win32": f"{stem}.dll"}
    name = next(v for k, v in names.items() if sys.platform.startswith(k))
    # single-config generators put the lib at the build root; MSVC nests it under a config dir
    for cand in (_BUILD / name, _BUILD / "Release" / name, _BUILD / "Debug" / name):
        if cand.exists():
            return cand
    return _BUILD / name


def _build_lib() -> None:
    subprocess.run(["cmake", "-B", str(_BUILD), "-S", str(KERNEL / "tools" / "capi")],
                   cwd=KERNEL, check=True, capture_output=True)
    subprocess.run(["cmake", "--build", str(_BUILD), "--config", "Release", "--parallel"],
                   cwd=KERNEL, check=True, capture_output=True)


def load() -> ctypes.CDLL:
    if not _lib_path().exists():
        print("building taptools_capi ...")
        _build_lib()
    lib = ctypes.CDLL(str(_lib_path()))

    vp = ctypes.c_void_p
    f32p = ctypes.POINTER(ctypes.c_float)
    f64p = ctypes.POINTER(ctypes.c_double)
    sigs = {
        "taptools_conv_create":         ([], vp),
        "taptools_conv_destroy":        ([vp], None),
        "taptools_conv_configure":      ([vp, ctypes.c_int, ctypes.c_int], ctypes.c_int),
        "taptools_conv_load_ir":        ([vp, f32p, f32p, f32p, f32p, ctypes.c_int, ctypes.c_double],
                                         ctypes.c_int),
        "taptools_conv_clear":          ([vp], ctypes.c_int),
        "taptools_conv_process":        ([vp, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_conv_block_size":     ([vp], ctypes.c_int),
        "taptools_conv_max_partitions": ([vp], ctypes.c_int),
        "taptools_conv_has_ir":         ([vp], ctypes.c_int),
    }
    for name, (argtypes, restype) in sigs.items():
        fn = getattr(lib, name)
        fn.argtypes = argtypes
        fn.restype = restype
    return lib


_LIB = load()


def _f32(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float32)


def _f64(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float64)


def _p32(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_float))


def _p64(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_double))


def _check(code: int, who: str) -> None:
    if code is not None and code < 0:
        raise RuntimeError(f"{who} failed")


class Convolver:
    """A live tap.convolve~ conv_engine — true-stereo uniformly-partitioned overlap-save.

    >>> c = Convolver(blocksize=256, max_partitions=64)
    >>> c.load_ir(ll=ir, rr=ir)          # any of ll/lr/rl/rr; missing paths are silent
    >>> outL, outR = c.process(inL, inR) # wet (fully convolved) stereo output
    """

    def __init__(self, blocksize: int, max_partitions: int):
        self._h = _LIB.taptools_conv_create()
        if not self._h:
            raise RuntimeError("taptools_conv_create failed")
        _check(_LIB.taptools_conv_configure(self._h, int(blocksize), int(max_partitions)), "configure")

    def load_ir(self, ll=None, lr=None, rl=None, rr=None, scale: float = 1.0) -> None:
        """Publish the four true-stereo IR paths (LL, LR, RL, RR). Missing paths are silent; the
        arrays may differ in length (shorter ones are zero-padded to the longest)."""
        arrs = [ll, lr, rl, rr]
        length = max((len(a) for a in arrs if a is not None), default=0)
        self._ir_keep = []  # keep the float buffers alive across the C call
        ptrs = []
        for a in arrs:
            if a is None:
                ptrs.append(None)
            else:
                f = _f32(np.pad(np.asarray(a, dtype=np.float32), (0, length - len(a))))
                self._ir_keep.append(f)
                ptrs.append(_p32(f))
        _check(_LIB.taptools_conv_load_ir(self._h, ptrs[0], ptrs[1], ptrs[2], ptrs[3],
                                          int(length), float(scale)), "load_ir")

    def process(self, inL, inR) -> tuple[np.ndarray, np.ndarray]:
        inL, inR = _f64(inL), _f64(inR)
        n = len(inL)
        outL, outR = np.empty(n), np.empty(n)
        _check(_LIB.taptools_conv_process(self._h, _p64(inL), _p64(inR), _p64(outL), _p64(outR), n),
               "process")
        return outL, outR

    def clear(self) -> None:
        _check(_LIB.taptools_conv_clear(self._h), "clear")

    @property
    def block_size(self) -> int:
        return _LIB.taptools_conv_block_size(self._h)

    @property
    def max_partitions(self) -> int:
        return _LIB.taptools_conv_max_partitions(self._h)

    @property
    def has_ir(self) -> bool:
        return bool(_LIB.taptools_conv_has_ir(self._h))

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_conv_destroy(h)
            self._h = None


def convolve_stereo(inL, inR, ll=None, lr=None, rl=None, rr=None, *, blocksize: int = 256,
                    scale: float = 1.0) -> tuple[np.ndarray, np.ndarray]:
    """One-shot helper: run inL/inR through a fresh engine with the given IR paths, sized to hold
    the whole IR. Returns the wet stereo output (latency = blocksize samples)."""
    length = max((len(a) for a in (ll, lr, rl, rr) if a is not None), default=0)
    max_parts = max(1, (length + blocksize - 1) // blocksize)
    c = Convolver(blocksize, max_parts)
    c.load_ir(ll=ll, lr=lr, rl=rl, rr=rr, scale=scale)
    return c.process(inL, inR)
