# Campos
<!-- keywords: campo, campos, atualizar campo, F9, desligar, bloquear, códigos de campo, número de página, data, NUMPAGES, SEQ, REF, PAGEREF, fórmula, Inserir Campo -->

Um **campo** é um texto que o programa calcula por si, como um número de página, a data de hoje ou o número de uma figura. Tem duas partes: um **código**, que indica o que calcular, e um **resultado**, que é o texto que vê. O código de um número de página é `PAGE` e o respetivo resultado na terceira página é `3`.

## Inserir um campo no texto

1. Clique onde pretende colocar o campo.
2. Clique em **Inserir > Campo**.
3. Escolha um campo na lista. O código aparece na caixa abaixo e pode alterá-lo.
4. Clique em **OK**.

Por exemplo, escolha **Data**, altere o código para `DATE \@ "d 'de' MMMM 'de' yyyy"` e clique em **OK**. O texto mostra a data como `2 de novembro de 2026`.

## Atualizar campos

Um campo mostra o resultado que tinha quando foi calculado pela última vez. Prima **F9** para voltar a calcular o campo onde está o cursor. Para atualizar vários campos de uma vez, selecione o texto que os contém e prima **F9**. Prima **Ctrl+A** e depois **F9** para atualizar todos os campos do documento.

A atualização é um único passo: **Anular** repõe todos os resultados. A atualização não é uma alteração controlada, mesmo com o controlo de alterações ativado, porque o resultado não é algo que tenha escrito.

Um campo num cabeçalho ou rodapé que mostre o número de página, o número total de páginas ou um título está sempre correto e não precisa de ser atualizado.

## Mostrar os códigos

Prima **Shift+F9** para mostrar o código do campo onde está o cursor, entre chavetas, como `{ PAGE }`, em vez do resultado. Prima **Alt+F9** para mostrar os códigos de todos os campos e prima novamente para mostrar os resultados. Isto altera apenas o que vê: o ficheiro é guardado tal como estava e o documento não é marcado como alterado.

## Desligar ou bloquear um campo

- **Ctrl+Shift+F9** converte o campo em texto simples. O resultado mantém-se e deixa de ser um campo.
- **Ctrl+F11** bloqueia um campo, para que **F9** não o altere. **Ctrl+Shift+F11** desbloqueia-o.

## Formatos

Adicione opções ao código para alterar o aspeto do resultado:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` alteram as maiúsculas/minúsculas ou o tipo de número. `{ = 21 \* CardText }` mostra `vinte e um`.
- `\# "0,00"` define o aspeto de um número. `{ = 1234,5 \# "# ##0,00" }` mostra `1 234,50`.
- `\@ "dddd, d 'de' MMMM"` define o aspeto de uma data. `M` é o mês e `m` é o minuto.

## Campos que o JeffWrite calcula

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, QUOTE, uma fórmula que comece por `=` e TOC. Um campo de qualquer outro tipo mantém o resultado que tem e é guardado tal como estava.

DATE, TIME, FILENAME e AUTHOR dependem do computador e do momento, pelo que podem mudar sempre que atualizar.
