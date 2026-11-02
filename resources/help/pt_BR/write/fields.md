# Campos
<!-- keywords: campo, campos, atualizar campo, F9, desvincular, bloquear, códigos de campo, número de página, data, NUMPAGES, SEQ, REF, PAGEREF, fórmula, Inserir Campo -->

Um **campo** é um trecho de texto que o programa calcula para você, como um número de página, a data de hoje ou o número de uma figura. Ele tem duas partes: um **código**, que diz o que calcular, e um **resultado**, que é o texto que você vê. O código de um número de página é `PAGE`, e o resultado dele na terceira página é `3`.

## Inserir um campo no texto

1. Clique onde o campo deve ficar.
2. Clique em **Inserir > Campo**.
3. Escolha um campo na lista. O código dele aparece na caixa abaixo, e você pode alterá-lo.
4. Clique em **OK**.

Por exemplo, escolha **Data**, altere o código para `DATE \@ "d 'de' MMMM 'de' yyyy"` e clique em **OK**. O texto mostra a data como `2 de novembro de 2026`.

## Atualizar campos

Um campo mostra o resultado que tinha quando foi calculado pela última vez. Pressione **F9** para calcular novamente o campo em que o cursor está. Para atualizar vários campos de uma vez, selecione o texto que os contém e pressione **F9**. Pressione **Ctrl+A** e depois **F9** para atualizar todos os campos do documento.

A atualização é uma única etapa: **Desfazer** restaura todos os resultados anteriores. A atualização não é uma alteração controlada, mesmo quando Controlar Alterações está ativado, porque o resultado não é algo que você digitou.

Um campo em um cabeçalho ou rodapé que mostra o número da página, a contagem de páginas ou um título está sempre correto e não precisa ser atualizado.

## Mostrar os códigos

Pressione **Shift+F9** para mostrar, no lugar do resultado, o código do campo em que o cursor está, entre chaves, como `{ PAGE }`. Pressione **Alt+F9** para mostrar os códigos de todos os campos e pressione novamente para mostrar os resultados. Isso só altera o que você vê: o arquivo é salvo como estava, e o documento não é marcado como alterado.

## Desvincular ou bloquear um campo

- **Ctrl+Shift+F9** transforma o campo em texto simples. O resultado permanece, e ele deixa de ser um campo.
- **Ctrl+F11** bloqueia um campo, para que **F9** não o altere. **Ctrl+Shift+F11** o desbloqueia.

## Formatos

Adicione opções ao código para alterar a aparência de um resultado:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` alteram as maiúsculas e minúsculas ou o tipo de número. `{ = 21 \* CardText }` mostra `vinte e um`.
- `\# "0,00"` define a aparência de um número. `{ = 1234,5 \# "#.##0,00" }` mostra `1.234,50`.
- `\@ "dddd, d 'de' MMMM"` define a aparência de uma data. `M` é o mês, e `m` é o minuto.

## Campos que o JeffWrite calcula

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, QUOTE, uma fórmula que começa com `=` e TOC. Um campo de qualquer outro tipo mantém o resultado que tem e é salvo como estava.

DATE, TIME, FILENAME e AUTHOR dependem do computador e do momento, por isso podem mudar sempre que você atualiza.
