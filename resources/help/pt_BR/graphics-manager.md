# Gerenciador de Elementos Gráficos
<!-- keywords: gerenciador de elementos gráficos, imagens, figuras, fotos, imagem vinculada, vincular ao arquivo, atualizar vínculo, alterar vínculo, inserir imagem, imagem inserida, incorporada, ausente, faltando, modificada, não atualizada, fora da pasta, revincular, localizar arquivo de imagem, status da imagem, resolução -->

O **Gerenciador de Elementos Gráficos** lista todas as imagens da publicação e informa a origem de cada uma. Ele é mais importante para as imagens vinculadas aos seus arquivos, porque uma imagem vinculada depende de um arquivo fora da publicação. Para saber como vincular uma imagem, consulte [Adicionar e ajustar imagens](pictures).

## Abrir o painel

Clique em **Exibir > Mostrar > Gerenciador de Elementos Gráficos**. O painel é aberto à direita. Para fechá-lo, clique novamente no mesmo botão ou no **X** na parte superior do painel.

## Ler a lista

Cada linha mostra uma miniatura, o nome do arquivo, a página, o tipo de arquivo, o tamanho em KB e o **status**. A caixa acima da lista classifica as linhas por página, nome, tamanho ou tipo. Abaixo da lista, os detalhes da imagem selecionada informam o status, o arquivo, se a publicação guarda a imagem inteira, o tamanho em pixels e a **resolução efetiva**: quantos pixels da imagem cabem em cada polegada da página. Cerca de 300 é um bom valor para impressão.

| Status | O que significa |
|---|---|
| Inserida | A imagem está armazenada dentro da publicação. Nada de fora é necessário. |
| Vinculada | A imagem está ligada a um arquivo, e o arquivo está onde o JeffPub espera, sem alterações. |
| Ausente | A imagem está ligada a um arquivo que não existe mais. Se a publicação não guarda nenhuma cópia, a página mostra a pequena visualização, que fica desfocada, e é ela que será impressa. |
| Modificada | O arquivo foi alterado desde que o vínculo foi criado ou atualizado pela última vez. Clique em **Atualizar Vínculo** para trazer as alterações. |
| Não atualizada | A imagem está ligada a um arquivo fora da pasta da publicação, por isso o JeffPub não abriu esse arquivo. A página mostra o que a publicação armazenou. Clique em **Atualizar Vínculo** para usar o arquivo. Consulte "Imagens de fora da pasta da publicação", abaixo. |

## O que os botões fazem

- **Ir para** seleciona a imagem na página dela.
- **Salvar Como…** salva uma cópia da imagem em um arquivo.
- **Substituir…** coloca um arquivo diferente na mesma moldura.
- **Atualizar Vínculo** lê o arquivo novamente. Use-o para uma imagem **Modificada**, depois de colocar de volta um arquivo **Ausente** ou para uma imagem **Não atualizada**. Todas as imagens que compartilham o vínculo são atualizadas, e a imagem mantém a posição e a largura.
- **Alterar Vínculo…** aponta a imagem para outro arquivo, por exemplo, quando você moveu os arquivos para outra pasta.
- **Inserir Imagem** transforma o vínculo em uma cópia armazenada na publicação. A imagem passa a aparecer como **Inserida**. Se o arquivo estiver ausente, somente a pequena visualização poderá ser inserida, e a barra de status avisará isso.

Os três botões de vínculo ficam acinzentados, a menos que a imagem selecionada seja vinculada. Cada um é uma única etapa, então **Ctrl+Z** a desfaz.

## Imagens de fora da pasta da publicação

Ao abrir uma publicação, o JeffPub lê os arquivos das imagens vinculadas que estão na mesma pasta da publicação ou em uma pasta dentro dela. Ele não procura arquivos em nenhum outro lugar, nem no seu computador nem na rede.

O motivo: uma publicação que alguém envia para você pode indicar qualquer arquivo. Se o JeffPub abrisse tudo o que a publicação indica, a publicação de um desconhecido poderia trazer uma das suas imagens particulares para as páginas dela, e você poderia salvá-la ou enviá-la de volta sem perceber. Em uma rede, apenas verificar se um arquivo existe também pode enviar suas credenciais de entrada do Windows para outro computador.

Por isso, uma imagem vinculada a um arquivo em outro lugar mostra o que a publicação armazenou: a imagem inteira, se a publicação guardar uma, ou a pequena visualização, que fica desfocada. O status é **Não atualizada**, e os detalhes informam que o arquivo está fora da pasta.

Para usar o arquivo, selecione a imagem e clique em **Atualizar Vínculo**, ou clique em **Alterar Vínculo…** e escolha o arquivo você mesmo. Essa é a sua autorização para aquela imagem, e ela vale até você fechar a publicação. As outras imagens continuam **Não atualizadas** até você fazer o mesmo com cada uma.

Ao salvar, o vínculo é mantido como estava, então, na próxima vez que você abrir a publicação, a imagem estará **Não atualizada** novamente. Se quiser que as imagens sejam carregadas automaticamente, mantenha-as na pasta da publicação ou em uma pasta dentro dela.

Imprimir, salvar um PDF e salvar um e-book usam o que a página mostra. Se uma imagem estiver **Não atualizada**, clique em **Atualizar Vínculo** antes de imprimir. O [Verificador de design](design-checker) lista essas imagens.

## Corrigir uma imagem ausente

1. Selecione a imagem **Ausente** na lista.
2. Clique em **Alterar Vínculo…** e escolha o arquivo. Ou mova o arquivo de volta para onde estava e clique em **Atualizar Vínculo**.
3. Se o arquivo não existir mais, clique em **Inserir Imagem** para manter a visualização ou em **Substituir…** para usar outra imagem.

O [Verificador de design](design-checker) também lista as imagens vinculadas que estão ausentes.

## Tópicos relacionados

- [Adicionar e ajustar imagens](pictures)
- [Verificar o design](design-checker)
- [Salvar como imagem, página da Web ou e-book](export)
