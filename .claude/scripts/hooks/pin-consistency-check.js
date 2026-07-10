#!/usr/bin/env node
'use strict';

/**
 * Hook PostToolUse (matcher: Edit|Write).
 * Advierte si el archivo .cpp/.h recién editado desincroniza los pines
 * respecto a diagram.json. Nunca bloquea: siempre exit 0.
 */

const path = require('path');
const fs = require('fs');

function readStdin() {
  try {
    return fs.readFileSync(0, 'utf8');
  } catch (err) {
    return '';
  }
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

  let checkPinConsistency;
  try {
    ({ checkPinConsistency } = require(path.join(__dirname, '..', 'lib', 'pins.js')));
  } catch (err) {
    return;
  }

  const root = input.cwd || process.cwd();

  let result;
  try {
    result = checkPinConsistency(root);
  } catch (err) {
    return;
  }

  if (result && result.ok === false) {
    console.error('[pin-consistency-check] Posible desincronización de pines entre el código y diagram.json:');
    (result.enCodigoNoEnDiagrama || []).forEach((line) => {
      console.error(`  - En código pero no conectado en diagram.json: ${line}`);
    });
    (result.enDiagramaNoEnCodigo || []).forEach((line) => {
      console.error(`  - Conectado en diagram.json pero no referenciado en el código: ${line}`);
    });
    console.error('[pin-consistency-check] Solo advertencia, no bloquea. Revisa con /pin-check.');
  }
}

try {
  main();
} catch (err) {
  // Nunca bloquear por un error inesperado del propio hook.
}

process.exit(0);
