# Regras para o conteúdo de uma célula
<!-- keywords: validação, regra, validação de dados, lista, lista pendente, pendente, número inteiro, decimal, data, hora, comprimento, personalizada, parar, aviso, informação, alerta de erro, não permitido, opções, escolhas, seta, Alt+Seta para Baixo -->

Um livro pode ter **regras** que indicam o que pode ser introduzido em determinadas células: um número inteiro de 1 a 10, uma data posterior a um dado dia, um texto com no máximo cinco letras ou uma opção de uma lista. As regras vêm do livro; o JeffCrunch aplica-as quando **escreve** um valor numa célula (ou escolhe um da respetiva lista). Os valores que cola, preenche ou move para a célula não são verificados, tal como nos programas de folha de cálculo habituais, pelo que uma regra indica o que pode ser escrito e não o que a célula contém.

## Quando um valor não cumpre a regra

Cada regra tem um **estilo**, e a mensagem é a da própria regra quando o livro a tem:

- **Parar**: a mensagem aparece e a entrada fica na célula para que a altere. Prima **Esc** para desistir.
- **Aviso**: é-lhe perguntado se pretende manter o valor mesmo assim.
- **Informação**: é informado de que o valor não cumpre a regra e pode continuar ou desistir.

Um valor colado não está sujeito à regra: é introduzido tal como está e nada é perguntado. Limpar uma célula é sempre permitido.

## Escolher a partir de uma lista

Uma célula com uma lista mostra um pequeno botão com uma seta no limite direito quando tem o cursor. Clique no botão, ou prima **Alt+Seta para Baixo**, para ver as opções e clique numa para a introduzir.

## O que não é feito

O JeffCrunch mantém as regras que o livro tem, mas ainda não tem nenhum comando para criar novas. As regras que leem as opções de outra folha na forma de escrita mais recente são mantidas no ficheiro, mas não são aplicadas. As mensagens que aparecem quando seleciona uma célula não são mostradas.
