#!/usr/bin/env node
'use strict';

/**
 * Hook PostToolUse (matcher: Edit|Write).
 * Advierte si el archivo .cpp/.h recién editado contiene delay() fuera
 * del cuerpo de setup(). Nunca bloquea: siempre exit 0.
 */

const fs = require('fs');
const path = require('path');

function readStdin() {
  try {
    return fs.readFileSync(0, 'utf8');
  } catch (err) {
    return '';
  }
}

// Encuentra el rango [inicioLlave, finLlave] del cuerpo de una función void <name>(...) { ... }
function findFunctionRange(text, name) {
  const startRegex = new RegExp(`\\bvoid\\s+${name}\\s*\\([^)]*\\)\\s*\\{`);
  const m = startRegex.exec(text);
  if (!m) return null;

  const braceStart = text.indexOf('{', m.index);
  if (braceStart === -1) return null;

  let depth = 0;
  for (let i = braceStart; i < text.length; i++) {
    if (text[i] === '{') depth++;
    else if (text[i] === '}') {
      depth--;
      if (depth === 0) return [braceStart, i];
    }
  }
  return [braceStart, text.length];
}

function findDelayOutsideSetup(text) {
  const setupRange = findFunctionRange(text, 'setup');
  const delayRegex = /\bdelay\s*\(/g;
  const lines = [];
  let match;
  while ((match = delayRegex.exec(text)) !== null) {
    const insideSetup = setupRange && match.index >= setupRange[0] && match.index <= setupRange[1];
    if (!insideSetup) {
      const lineNumber = text.slice(0, match.index).split('\n').length;
      lines.push(lineNumber);
    }
  }
  return lines;
}

function main() {
  const raw = readStdin();

  let input = {};
  try {
    input = JSON.parse(raw);
  } catch (err) {
    return;
  }

  const filePath = input.tool_input && input.tool_input.file_path;
  if (!filePath || !/\.(cpp|h|hpp|cc)$/i.test(filePath)) {
    return;
  }

  let text;
  try {
    text = fs.readFileSync(filePath, 'utf8');
  } catch (err) {
    return;
  }

  const lines = findDelayOutsideSetup(text);
  if (lines.length) {
    const rel = path.basename(filePath);
    console.error(`[no-delay-guard] ${rel}: delay() fuera de setup() en línea(s) ${lines.join(', ')}.`);
    console.error('[no-delay-guard] Esto bloquea el loop principal y puede retrasar una alarma de emergencia. Usa millis() (ver skill arduino-embedded-standards). Solo advertencia.');
  }
}

try {
  main();
} catch (err) {
  // Nunca bloquear por un error inesperado del propio hook.
}

process.exit(0);
