#!/usr/bin/env node
'use strict';

/**
 * Hook Stop.
 * Si en la sesión se editaron archivos .cpp/.h/.ini y no se detecta una
 * ejecución de "pio run" después de esas ediciones, imprime un recordatorio.
 * Nunca bloquea: siempre exit 0.
 */

const fs = require('fs');

function readStdin() {
  try {
    return fs.readFileSync(0, 'utf8');
  } catch (err) {
    return '';
  }
}

function extractToolUses(transcriptText) {
  const toolUses = [];
  const lines = transcriptText.split('\n').filter(Boolean);
  for (const line of lines) {
    let entry;
    try {
      entry = JSON.parse(line);
    } catch (err) {
      continue;
    }
    const content = entry && entry.message && entry.message.content;
    if (!Array.isArray(content)) continue;
    for (const item of content) {
      if (item && item.type === 'tool_use' && item.name) {
        toolUses.push({ name: item.name, input: item.input || {} });
      }
    }
  }
  return toolUses;
}

function main() {
  const raw = readStdin();

  let input = {};
  try {
    input = JSON.parse(raw);
  } catch (err) {
    return;
  }

  const transcriptPath = input.transcript_path;
  if (!transcriptPath || !fs.existsSync(transcriptPath)) {
    return;
  }

  let transcriptText;
  try {
    transcriptText = fs.readFileSync(transcriptPath, 'utf8');
  } catch (err) {
    return;
  }

  const toolUses = extractToolUses(transcriptText);

  let lastPioRunIndex = -1;
  toolUses.forEach((tu, idx) => {
    if (tu.name === 'Bash' && typeof tu.input.command === 'string' && /\bpio\s+run\b/.test(tu.input.command)) {
      lastPioRunIndex = idx;
    }
  });

  const editedAfter = new Set();
  toolUses.forEach((tu, idx) => {
    const isEdit = tu.name === 'Edit' || tu.name === 'Write' || tu.name === 'MultiEdit';
    const filePath = tu.input.file_path;
    if (isEdit && typeof filePath === 'string' && /\.(cpp|h|hpp|cc|ini)$/i.test(filePath) && idx > lastPioRunIndex) {
      editedAfter.add(filePath);
    }
  });

  if (editedAfter.size) {
    console.error(`[pio-build-reminder] Se editaron ${editedAfter.size} archivo(s) de firmware/config sin correr "pio run" después: ${[...editedAfter].join(', ')}`);
    console.error('[pio-build-reminder] Recuerda ejecutar /build (o "pio run") antes de simular o subir a la placa. Solo advertencia.');
  }
}

try {
  main();
} catch (err) {
  // Nunca bloquear por un error inesperado del propio hook.
}

process.exit(0);
