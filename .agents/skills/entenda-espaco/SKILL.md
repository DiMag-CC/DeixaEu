---
name: entenda-espaco
description: Use this skill when the user asks to "entenda esse espaco", wants a full workspace walkthrough, or asks for a structural reading of the currently open directory. This skill inspects the project tree, reads the important source and configuration files, identifies the main modules and flows, and explains the project's logic before proposing or making changes.
---

# Entenda Espaco

## Overview

Use this skill when the user wants Codex to first understand the currently open project space before answering questions, reviewing code, or implementing changes.

Trigger this skill when the request includes the exact phrase `entenda esse espaco` or an explicit invocation such as `/entenda-espaco`.

## Workflow

1. Identify the project root from the current workspace and inspect the directory tree.
2. Read the key files that explain the project first:
- repository metadata and lockfiles
- build and runtime configuration
- entrypoints
- source directories
- test directories
- docs that affect behavior
3. Prefer broad coverage with sensible filtering:
- include source files, configs, manifests, migrations, schemas, and local docs
- skip obvious generated or dependency-heavy directories unless they are directly relevant, such as `node_modules`, `dist`, `build`, cache folders, binary assets, and large vendor bundles
4. Build a mental model of:
- the app purpose
- the main modules and ownership boundaries
- data flow
- control flow
- external integrations
- build/test/run workflow
5. Return a compact explanation in plain language:
- what this project is
- how it is organized
- where the core logic lives
- what to read first for future changes
- any uncertainty or gaps that still need confirmation

## Reading Strategy

- Start with fast tree discovery using `rg --files`, `find`, `ls`, and targeted reads.
- Read high-signal files before deep-diving into every implementation detail.
- When the repository is small, it is reasonable to read nearly all text files.
- When the repository is large, cover the whole structure but prioritize files that define behavior and architecture.
- If multiple apps or packages exist, map each one separately and then explain how they connect.

## Expected Output

The response should usually contain:

- a short project summary
- the directory or module map
- the main execution path
- the important files and why they matter
- notable conventions, risks, or confusing areas

## Notes

- Do not claim certainty when files were skipped due to size, binary format, or obvious generated output.
- If the user asks for changes after this reading pass, use the project map you built as the basis for the next step.
