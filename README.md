# vicon_transformer

Wrapper around the [Vicon Datastream SDK](https://www.vicon.com/software/datastream-sdk)
that simplifies setup and access to the data.

Most relevantly, the `ViconTransformer` class can provide poses of all subjects relative
to a configurable "origin subject".  This origin subject should be fixed in the scene
(e.g. screwed to a wall), such that it cannot move over time.  It then serves as a
clearly defined origin of the scene, that is independent of the actual origin used by
Vicon (which otherwise would need to be redefined after every re-calibration and is
generally difficult to set precisely).

The implementation is kept generic, such that it should work for different applications.

The package also contains templated
[o80](https://github.com/intelligent-soft-robots/o80) driver and standalone classes,
which can be specialised for specific applications by providing the number of subjects
and a name-to-index mapping function.  See pam_vicon_o80 for an example.


For more information see the
**[documentation](https://intelligent-soft-robots.github.io/vicon_transformer)**.


# pam_vicon_o80

NOTE: This part will be moved to its own package soon.

o80 driver and standalone implementations for the ISR-PAM Vicon system.
