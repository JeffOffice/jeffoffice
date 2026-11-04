# Campos

<!-- keywords: campo, campos, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, atualizar campo, F9, desvincular, bloquear, códigos de campo, número de página, data, NUMPAGES, SEQ, REF, PAGEREF, fórmula, Inserir Campo -->

Um **campo** é um trecho de texto que o programa calcula para você, como um número de página, a data de hoje ou o número de uma figura. Ele tem duas partes: um **código**, que diz o que calcular, e um **resultado**, que é o texto que você vê. O código de um número de página é `PAGE`, e o resultado na terceira página é `3`.

## Inserir um campo no texto

1. Clique onde o campo deve ficar.
2. Clique em **Inserir > Campo**.
3. Escolha um campo na lista. O código dele aparece na caixa abaixo, e você pode alterá-lo.
4. Clique em **OK**.

Por exemplo, escolha **Data**, altere o código para `DATE \@ "MMMM d, yyyy"` e clique em **OK**. O texto mostra a data como `2 de novembro de 2026`.

## Atualizar campos

Um campo mostra o resultado que tinha da última vez em que foi calculado. Pressione **F9** para calcular de novo o campo onde está o cursor. Para atualizar vários campos de uma vez, selecione o texto que os contém e pressione **F9**. Pressione **Ctrl+A** e depois **F9** para atualizar todos os campos do documento.

A atualização é uma única etapa: **Desfazer** restaura todos os resultados anteriores. A atualização não é uma alteração controlada, mesmo com Controlar Alterações ativado, porque o resultado não é algo que você digitou.

Um campo em um cabeçalho ou rodapé que mostra o número da página, o total de páginas ou um título está sempre correto e não precisa de atualização.

## Mostrar os códigos

Pressione **Shift+F9** para mostrar, no lugar do resultado, o código do campo onde está o cursor, entre chaves, como `{ PAGE }`. Pressione **Alt+F9** para mostrar os códigos de todos os campos e pressione novamente para voltar a mostrar os resultados. Isso altera apenas o que você vê: o arquivo é salvo como estava, e o documento não é marcado como alterado.

## Desvincular ou bloquear um campo

- **Ctrl+Shift+F9** transforma o campo em texto comum. O resultado permanece, mas ele deixa de ser um campo.
- **Ctrl+F11** bloqueia um campo, para que **F9** não o altere. **Ctrl+Shift+F11** desbloqueia.

## Formatos

Acrescente opções ao código para mudar a aparência de um resultado:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` mudam o uso de maiúsculas ou o tipo de número. `{ = 21 \* CardText }` mostra `vinte e um`.
- `\# "0.00"` define a aparência de um número. `{ = 1234.5 \# "#,##0.00" }` mostra `1,234.50`.
- `\@ "dddd, MMMM d"` define a aparência de uma data. `M` é o mês, e `m` é o minuto.

## Campos que o JeffWrite calcula

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, uma fórmula que começa com `=`, TOC (também um índice de ilustrações), INDEX, TOA, CITATION e BIBLIOGRAPHY. As marcas ocultas TC, XE e TA não têm resultado a calcular: elas indicam a uma tabela, a um índice remissivo ou a um índice de autoridades o que deve ser listado. Um campo de qualquer outro tipo mantém o resultado que tem e é salvo como estava.

DATE, TIME, FILENAME e AUTHOR dependem do computador e do momento, por isso podem mudar a cada atualização.
