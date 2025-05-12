# Firecracker tools

Here resides all that is necessary to configure and run Firecracker microVMs along with user generated "functions" to be
run inside these microVMs which will be managed by the host process [`microDNS`](../dns/README.md).
Each subdirectory has its own `README` to explain its purpose.

* `vm_config.json` is a Firecracker launch configuration that avoids configuring it via its socket HTTP interface when
trying to launch a microVM. This one is not a valid JSON config but instead a template for `microDNS` to dynamically
populate when launching a Firecracker process.
* `vm_test_config.json` is the same as above except that it is valid and used when launching a microVM via `make test` in an interactive session. It has more resources assigned as well as boots with a proper init system.
