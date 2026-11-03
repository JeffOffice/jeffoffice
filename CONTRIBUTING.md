# Contributing to JeffOffice

Thank you for helping. This page says how contributions are accepted. Contributions are welcome.

## The rule: sign off every commit

JeffOffice uses the **Developer Certificate of Origin (DCO)**, version 1.1. By adding a sign-off line to a commit, you certify the statements below. Add it with:

```
git commit -s -m "Fix the page margin check"
```

This adds a line at the end of the commit message:

```
Signed-off-by: Your Name <you@example.com>
```

Use your legal name, or the name you are publicly known by, and an email address you can be reached at. Anonymous sign-offs are not accepted. To sign off commits you already made, run `git commit --amend -s` (the last one) or `git rebase --signoff` (several). A pull request with unsigned commits will not be merged. An automatic check on every pull request looks for the sign-off.

## Developer Certificate of Origin, Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.

Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.

```
Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

## What this means for you

- **Your code stays yours.** You keep the copyright in your contribution. JeffOffice LLC does not take ownership.
- **The license is the project's:** the GNU GPL version 3, with the project's Additional Terms (ADDITIONAL-TERMS.md). By signing off, you agree that your contribution is shared under the License together with those Additional Terms, and you authorize JeffOffice LLC to apply them to it.
- **Provenance.** We keep a record of who contributed what, and that each person says they have the right to share it. This protects you, other users, and JeffOffice LLC.

## What we ask you not to submit

- Code, text, images, fonts, or data copied from a program or source whose license is not compatible with the GPL version 3, or whose license you cannot check.
- Anything from a proprietary product, including code, icons, templates, or fonts, and anything you have seen only under a confidentiality agreement.
- Text or code produced by an AI tool or other generator that you have not reviewed and understood. You may use such tools, but your sign-off covers the result: you certify that you have the right to submit it, and you are responsible for it as if you wrote it. Say in the pull request if a large part was generated.
- Personal data or secrets (passwords, keys, tokens, real customer data) in code, tests, or sample files.

## Where the rest of the rules live

Coding style, tests, and the build are in the repository's README and docs. Trademark rules for forks are in the Trademark Policy. Questions: info@jeffoffice.com.

## Reporting a security problem

Please do not open a public issue. Email info@jeffoffice.com with "Security" in the subject, what you found, and how to reproduce it. We will reply within 7 days, keep you informed, and credit you when the fix is released unless you prefer not to be named.
