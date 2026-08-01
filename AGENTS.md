<!-- codebase-memory:start -->
# Codebase Memory — Code Intelligence

This project uses **codebase-memory** (`cbm_*` tools) for structure, call graph, and impact — **not GitNexus**.

> If graph is missing/stale: `cbm_index_repository` on repo root (`mode=fast` ok for quick refresh; `full` when needed).

## Always Do

- **Prefer `cbm_search_graph` / `cbm_trace_path` / `cbm_get_architecture` over blind grepping** when navigating symbols, callers, or subsystems.
- **Before non-trivial edits**, use `cbm_trace_path` (inbound) or search + connected nodes to estimate blast radius; call out high fan-in hotspots.
- **Before committing multi-file behavior changes**, re-check with `cbm_search_code` / graph that only expected paths moved.
- For “how does X work?”, start with `cbm_get_architecture` or `cbm_search_graph({query: "..."})`, then `cbm_get_code_snippet` for bodies.

## Never Do

- Do **not** require GitNexus (`gitnexus_*`, `npx gitnexus analyze`, gitnexus:// resources).
- Do **not** rename symbols with naive find-replace across the tree without checking call graph via codebase-memory (or LSP).
- Do **not** invent module APIs that already exist nearby — search graph first.

## Tools quick map

| Task | Tool |
|------|------|
| Find functions/classes | `cbm_search_graph` |
| Callers / impact | `cbm_trace_path` direction=inbound |
| Architecture overview | `cbm_get_architecture` |
| Read symbol body | `cbm_get_code_snippet` |
| Text/grep + graph rank | `cbm_search_code` |
| Reindex | `cbm_index_repository` |

<!-- codebase-memory:end -->

## Agent skills

### Issue tracker

GitHub Issues (`ioOvOoi/RTXGI-UE-5.7-Plugin`). See `docs/agents/issue-tracker.md`.

### Triage labels

5 canonical roles: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context layout. See `docs/agents/domain.md`.
