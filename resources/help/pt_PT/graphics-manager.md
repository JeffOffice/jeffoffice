# Gestor de Gráficos
<!-- keywords: gestor de gráficos, imagens, fotografias, imagem ligada, ligar ao ficheiro, atualizar ligação, alterar ligação, incorporar imagem, incorporada, em falta, modificado, não atualizada, fora da pasta, voltar a ligar, localizar ficheiro de imagem, estado da imagem, resolução -->

O **Gestor de Gráficos** apresenta todas as imagens da publicação e indica a origem de cada uma. É mais importante para as imagens ligadas aos respetivos ficheiros, porque uma imagem ligada depende de um ficheiro exterior à publicação. Para saber como ligar uma imagem, consulte [Adicionar e ajustar imagens](pictures).

## Abrir o gestor

Clique em **Ver > Mostrar > Gestor de Gráficos**. O painel abre-se à direita. Clique novamente no mesmo botão, ou no **X** na parte superior do painel, para o fechar.

## Ler a lista

Cada linha mostra uma pequena imagem, o nome do ficheiro, a página, o tipo de ficheiro, o tamanho em KB e o **estado**. A caixa acima da lista ordena as linhas por página, nome, tamanho ou tipo. Por baixo da lista, os detalhes da imagem selecionada indicam o estado, o ficheiro, se a publicação guarda a imagem completa, o tamanho em píxeis e a **resolução efetiva**: quantos píxeis da imagem cabem em cada polegada da página. Cerca de 300 é um bom valor para impressão.

| Estado | O que significa |
|---|---|
| Incorporada | A imagem está guardada dentro da publicação. Não é necessário nada exterior. |
| Ligada | A imagem está associada a um ficheiro, e o ficheiro está onde o JeffPub o espera, sem alterações. |
| Em falta | A imagem está associada a um ficheiro que já não existe. Se a publicação não guardar nenhuma cópia, a página mostra a pequena pré-visualização, que parece desfocada, e é isso que é impresso. |
| Modificado | O ficheiro foi alterado desde que a ligação foi criada ou atualizada pela última vez. Clique em **Atualizar Ligação** para incorporar as alterações. |
| Não atualizada | A imagem está associada a um ficheiro fora da pasta da publicação, por isso o JeffPub não abriu esse ficheiro. A página mostra o que a publicação guardou. Clique em **Atualizar Ligação** para utilizar o ficheiro. Consulte "Imagens de fora da pasta da publicação", abaixo. |

## O que fazem os botões

- **Ir para** seleciona a imagem na respetiva página.
- **Guardar Como…** guarda uma cópia da imagem num ficheiro.
- **Substituir…** coloca um ficheiro diferente na mesma moldura.
- **Atualizar Ligação** lê novamente o ficheiro. Utilize-o numa imagem **Modificado**, depois de repor um ficheiro **Em falta** ou numa imagem **Não atualizada**. Todas as imagens que partilham a ligação são atualizadas, e a imagem mantém a posição e a largura.
- **Alterar Ligação…** associa a imagem a outro ficheiro, por exemplo, quando moveu os ficheiros para outra pasta.
- **Incorporar Imagem** transforma a ligação numa cópia guardada na publicação. A imagem passa a aparecer como **Incorporada**. Se o ficheiro estiver em falta, só é possível incorporar a pequena pré-visualização, e a barra de estado informa-o disso.

Os três botões de ligação ficam a cinzento, a menos que a imagem selecionada esteja ligada. Cada um corresponde a um único passo, por isso **Ctrl+Z** anula-o.

## Imagens de fora da pasta da publicação

Ao abrir uma publicação, o JeffPub lê os ficheiros das imagens ligadas que estão na mesma pasta da publicação ou numa pasta dentro dela. Não consulta ficheiros noutro local, seja no computador ou na rede.

O motivo: uma publicação que alguém lhe envie pode indicar qualquer ficheiro. Se o JeffPub abrisse tudo o que a publicação indicasse, a publicação de um desconhecido poderia colocar uma das suas imagens privadas nas suas páginas, que poderia depois guardar ou reenviar sem se aperceber. Numa rede, a simples verificação da existência de um ficheiro pode também enviar as suas credenciais de início de sessão do Windows para outro computador.

Por isso, uma imagem ligada a um ficheiro noutro local mostra o que a publicação guardou: a imagem completa, se a publicação a guardar, ou caso contrário a pequena pré-visualização, que parece desfocada. O estado é **Não atualizada**, e os detalhes indicam que o ficheiro está fora da pasta.

Para utilizar o ficheiro, selecione a imagem e clique em **Atualizar Ligação**, ou clique em **Alterar Ligação…** e escolha o ficheiro manualmente. Isso constitui a sua autorização para essa imagem e é válida até fechar a publicação. As restantes imagens continuam **Não atualizada** até fazer o mesmo para cada uma.

Ao guardar, a ligação mantém-se como estava, por isso, da próxima vez que abrir a publicação, a imagem volta a estar **Não atualizada**. Se quiser que as imagens sejam carregadas automaticamente, mantenha-as na pasta da publicação ou numa pasta dentro dela.

A impressão, a gravação de um PDF e a gravação de um livro eletrónico utilizam o que a página mostra. Se uma imagem estiver **Não atualizada**, clique em **Atualizar Ligação** antes de imprimir. O [Verificador de Estrutura](design-checker) apresenta estas imagens.

## Corrigir uma imagem em falta

1. Selecione a imagem **Em falta** na lista.
2. Clique em **Alterar Ligação…** e escolha o ficheiro. Em alternativa, mova o ficheiro de volta para onde estava e clique em **Atualizar Ligação**.
3. Se o ficheiro tiver desaparecido definitivamente, clique em **Incorporar Imagem** para manter a pré-visualização, ou em **Substituir…** para utilizar outra imagem.

O [Verificador de Estrutura](design-checker) também apresenta as imagens ligadas que estão em falta.

## Relacionados

- [Adicionar e ajustar imagens](pictures)
- [Verificar a estrutura](design-checker)
- [Guardar como imagem, página Web ou livro eletrónico](export)
