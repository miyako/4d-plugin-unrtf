![version](https://img.shields.io/badge/version-17%2B-3E8B93)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-unrtf)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-unrtf/total)

# 4d-plugin-unrtf

The UnRTF plugin converts RTF documents to another text format using a bundled copy of the GNU `unrtf` library. You give it an RTF document as a `Blob`; it returns an object describing whether the conversion succeeded and, if so, the converted text as a `Text` value.

| Command | Returns | Purpose |
|---|---|---|
| [`UnRTF`](#unrtf) | `Object` | Convert an RTF document (as a `Blob`) to HTML, plain text, or LaTeX. |

**Platforms:** macOS, Windows

---

## Requirements & platform notes

- The plugin needs its `.conf` files (`html.conf`, `text.conf`, `latex.conf`) to be present alongside the plugin at runtime: in the plugin bundle's `Resources` folder on macOS, and in a `Resources` folder next to the plugin's `.dll`/folder on Windows (the plugin locates this itself — you don't pass a path). If a `.conf` file is missing or unreadable, `UnRTF` returns `success: false` with no crash.
- **On Windows**, the plugin copies the input `Blob`'s bytes into a temporary file (via `tmpfile()`) before parsing it. **On macOS**, it parses the `Blob`'s memory directly (`fmemopen`), with no temp-file I/O. This is invisible to the calling 4D code, but Windows conversions of very large documents will touch temp storage where macOS conversions won't.
- `UnRTF` is declared `threadSafe` in the plugin's manifest, so it can be called from worker processes/preemptive code.
- The `codepage` option (see below) is passed straight through from your `options` object to the plugin's internal string-encoding conversion. I don't have the source for that specific conversion helper, so I can't confirm the exact encoding rules beyond "it's a Windows-style codepage identifier (e.g. `1252` for Windows Latin-1)" — if the conversion can't represent your text in the requested codepage, `UnRTF` reports `success: false` with `error: "bad encoding"` rather than truncating or crashing.

---

## UnRTF

### Syntax

```4d
$status := UnRTF ( RTF ; options )
```

| Parameter | Type | Description |
|---|---|---|
| `RTF` | Blob | The RTF document to convert, as raw bytes (e.g. from `Document.getContent()`). Mandatory — the command reads this parameter unconditionally. |
| `options` | Object | Conversion options (see below). Optional in practice: the plugin only reads from this object if it isn't null, and falls back to `format: "html"` and no codepage conversion if you omit it entirely — though none of the plugin's own sample code actually calls `UnRTF` without it, so this is inferred from the parameter-reading code, not demonstrated. |
| Result | Object | See the **Result object** table below. |

**`options` properties:**

| Property | Type | Description |
|---|---|---|
| `format` | Text | One of `"html"` (default), `"txt"`, or `"tex"`. Any other value — or omitting `format` — falls back to `"html"` silently; there's no error for an unrecognized format string. |
| `codepage` | Number | A numeric codepage identifier (e.g. `1252`) applied when encoding the result text. Omit it (or pass `0`) for the plugin's default encoding. |

**Result object:**

| Property | Type | Description |
|---|---|---|
| `success` | Boolean | `true` if conversion succeeded. |
| `result` | Text | Present only when `success` is `true`. The converted document. |
| `error` | Text | Present only when `success` is `false`. A short, human-readable reason. |

### Description

`UnRTF` parses the `RTF` blob with the bundled `unrtf` library and renders it using one of three built-in output personalities, selected by `options.format`:

- `"html"` — HTML output (the default if `format` is omitted or unrecognized).
- `"txt"` — plain text, formatting stripped.
- `"tex"` — LaTeX source.

A `"vt"` (VT100) output mode exists in the plugin's internal format enum but its selection branch is commented out in the current build — you cannot reach it through `options.format`, whatever string you pass.

If `RTF` is empty/null, if the plugin's `.conf` files can't be located or read, if the input can't be opened, or if the underlying RTF parser fails on malformed input, `UnRTF` returns an object with `success: false` and a descriptive `error` string rather than raising a 4D error or crashing.

> **Forward-looking note:** the specific wording above (`"missing input BLOB parameter"`, `"could not resolve plugin resources path"`, `"could not initialize output personality..."`, `"could not open input buffer"`, `"unknown error while parsing RTF"`) reflects a source-level fix applied during this review — guaranteeing every failure path sets a descriptive `error` and returns cleanly instead of occasionally leaving the host waiting. If you're running an older build of this plugin, some of these specific messages, and the guarantee that a result is always returned, may not yet apply; `"bad encoding"` is original, pre-existing behavior.

### Example

From the plugin's own test method (`TEST_rtf.4dm`):

```4d
$file:=Folder(fk desktop folder).file("test.rtf")

var $RTF : Blob

$RTF:=$file.getContent()

$options:=New object("format"; "txt"; "codepage"; 1252)

$status:=UnRTF($RTF; $options)
```

From the plugin's own test method (`TEST.4dm`), showing the generic success/error dispatch pattern used against `$status`:

```4d
C_TEXT($1; $format)

$format:=$1

var $RTF : Blob

$folder:=Folder(fk desktop folder).folder("UnRTF_"+$format)
$folder.delete(Delete with contents)
$folder.create()
$options:=New object("format"; $format)

For each ($file; Folder(fk resources folder).files(fk ignore invisible | fk recursive))
	
	If ($file.extension=".rtf")
		$RTF:=$file.getContent()
		$status:=UnRTF($RTF; $options)
		If ($status.success)
			$folder.file($file.name+"."+$format).setText($status.result)
		Else 
			ALERT($status.error)
		End if 
	End if 
	
End for each
```

`TEST_all.4dm` drives the method above across every supported format:

```4d
TEST("html")
TEST("txt")
TEST("tex")
```

A minimal, standalone variation converting a single file to HTML with no explicit `codepage`:

```4d
var $RTF : Blob
$RTF:=Folder(fk desktop folder).file("memo.rtf").getContent()

$status:=UnRTF($RTF; New object("format"; "html"))

If ($status.success)
	Folder(fk desktop folder).file("memo.html").setText($status.result)
Else
	ALERT($status.error)
End if
```

---

## Error handling & troubleshooting

- **`success` is always checked, never assumed.** Every conversion attempt — good input or bad — returns an object with a `success` key; there's no exception thrown into your 4D code. Always branch on `$status.success` before reading `$status.result`.
- **An unrecognized `format` value doesn't error — it silently produces HTML.** If you see HTML output when you expected `.txt` or `.tex`, check for a typo in the `format` string; there's no validation error for this.
- **`error: "bad encoding"` means the `codepage` you supplied couldn't represent the converted text**, not that the RTF itself was invalid. Try a different `codepage` (or omit it) rather than treating this as a parsing failure.
- **Missing or unreadable `.conf` files fail the whole conversion, not just formatting details.** This shows up as `success: false` with an error about resource/initialization — reinstalling the plugin bundle (so its `Resources` folder is intact) is the fix, not a code change on your end.
- **Very large or unusual RTF documents can still exhaust memory or time**, same as any parser — this plugin doesn't impose its own size limit on the input `Blob`.

---

## Quick reference

```4d
var $RTF : Blob
$RTF:=Folder(fk desktop folder).file("input.rtf").getContent()

$status:=UnRTF($RTF; New object("format"; "html"; "codepage"; 1252))

If ($status.success)
	Folder(fk desktop folder).file("output.html").setText($status.result)
Else
	ALERT($status.error)
End if
```
