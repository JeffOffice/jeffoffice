# Regras para o conteúdo de uma célula
<!-- keywords: validação, regra, validação de dados, lista, lista suspensa, menu suspenso, número inteiro, decimal, data, hora, comprimento, personalizada, parar, aviso, informações, alerta de erro, não permitido, opções, seta, Alt+Seta para baixo -->

Uma pasta de trabalho pode ter **regras** que definem o que pode ser colocado em algumas células: um número inteiro de 1 a 10, uma data posterior a um determinado dia, um texto de no máximo cinco letras ou uma opção de uma lista. As regras vêm da pasta de trabalho; o JeffCrunch as aplica quando você **digita** um valor em uma célula (ou escolhe um na lista dela). Valores que você cola, preenche ou move para a célula não são verificados, como nos programas de planilha que as pessoas conhecem; portanto, uma regra define o que pode ser digitado, e não o que a célula contém.

## Quando um valor não atende à regra

Cada regra tem um **estilo**, e a mensagem é a da própria regra quando a pasta de trabalho tem uma:

- **Parar**: a mensagem aparece e o valor digitado permanece na célula para você alterá-lo. Pressione **Esc** para desistir dele.
- **Aviso**: você é perguntado se deseja manter o valor mesmo assim.
- **Informações**: você é informado de que o valor não atende à regra e pode continuar ou desistir.

Um valor colado não precisa atender à regra: ele entra como está, e nada é perguntado. Limpar uma célula é sempre permitido.

## Escolher em uma lista

Uma célula que tem uma lista mostra um pequeno botão com uma seta na borda direita quando o cursor está nela. Clique no botão ou pressione **Alt+Seta para baixo** para ver as opções e clique em uma para inseri-la.

## O que não é feito

O JeffCrunch mantém as regras que a pasta de trabalho tem, mas ainda não tem um comando para criar novas regras. Regras que leem suas opções de outra planilha na forma mais recente de escrevê-las são mantidas no arquivo, mas não são aplicadas. As mensagens que aparecem quando você seleciona uma célula não são mostradas.
