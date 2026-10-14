# Security Policy

## Reporting a Vulnerability

Use GitHub's Security Advisories feature to report security concerns privately.

Expect a response within 7 days. If the issue is confirmed, a fix will be released as a patch version.

## Scope

`ucrc` is a small library with no network stack, no external dependencies, and no dynamic memory allocation. The primary attack surface is a buffer length that overstates the real buffer, and a model descriptor that breaks its documented contract. The library checks for a NULL model and NULL data, but it cannot discover the real length of an arbitrary pointer. A width other than 8, 16 or 32, or a table that does not match the model, is caller misuse.
