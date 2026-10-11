# Verificar a estrutura
<!-- keywords: verificador de estrutura, verificar, erros, problemas, baixa resolução, texto em excesso, texto não cabe, caixa de texto vazia, fora da página, verificação prévia, verificação final, prova, imagem ligada, tipo de letra em falta, fonte em falta -->

O **Verificador de Estrutura** analisa a publicação à procura de problemas frequentes. Encontra texto que não cabe, caixas vazias, objetos que saem da página e imagens que vão ficar mal impressas. Execute-o antes de imprimir ou de enviar um ficheiro para uma gráfica.

## Executar o Verificador de Estrutura

Clique em **Rever > Verificar > Executar o Verificador de Estrutura**. O painel **Verificador de Estrutura** abre-se à direita. Também pode clicar em **Ficheiro > Propriedades** e, em seguida, em **Executar o Verificador de Estrutura**.

O painel tem duas caixas de verificação, ambas ativadas inicialmente:

- **Executar verificações gerais de estrutura** procura problemas de esquema.
- **Executar verificações finais de publicação** procura aspetos importantes para a impressão.

Mantenha ambas ativadas para ver todos os tipos de problemas. A lista é atualizada à medida que trabalha. Se não houver nada a corrigir, é apresentada a mensagem "Não foram encontrados problemas."

## Corrigir um problema

1. Clique num item da lista em **Selecione um item para corrigir**.
2. Clique em **Ir para o Item**. Ou faça duplo clique no item.

O JeffPub vai para a página e seleciona o objeto. Corrija-o e o item desaparece da lista.

## O que encontra

| Mensagem | O que significa | O que fazer |
|---|---|---|
| A página não tem conteúdo | A página está vazia. | Adicione algo ou elimine a página. |
| O objeto está parcialmente fora da página | Parte de um objeto está fora da página. | Mova-o ou deixe-o ficar, se for uma sangria. |
| A caixa de texto está vazia | Uma caixa de texto não contém texto. | Escreva nela ou elimine-a. |
| Artigo com texto em excesso | Um artigo é o texto que percorre caixas ligadas. Este tem mais texto do que o que cabe. | Aumente uma caixa, corte texto ou [ligue outra caixa](linked-text). |
| O texto foi muito reduzido para caber | O ajuste automático, que reduz o texto para caber na caixa, deixou o texto com menos de metade do tamanho. | Aumente a caixa ou utilize menos texto. |
| O marcador de posição de imagem está vazio | Uma moldura de imagem não tem imagem. | Clique nela para adicionar uma ou elimine-a. |
| A imagem não está dimensionada proporcionalmente | A imagem está esticada ou comprimida. | Redimensione-a para repor a forma original. |
| A imagem está ligada, não incorporada | A imagem está associada a um ficheiro e apenas uma pequena pré-visualização é guardada na publicação. Uma imagem inserida com **Inserir e Ligar** é guardada na íntegra e não aparece na lista. | Mantenha o ficheiro original junto da publicação ou clique em **Incorporar Imagem** no [Gestor de Gráficos](graphics-manager). |
| A imagem ligada está em falta | O ficheiro para o qual a imagem ligada aponta já não existe, pelo que a página mostra apenas a pequena pré-visualização, e é isso que seria impresso. | No Gestor de Gráficos, clique em **Alterar Ligação…** para localizar o ficheiro ou reponha-o e clique em **Atualizar Ligação**. |
| A imagem ligada mostra apenas a pré-visualização até Atualizar Ligação | A imagem está ligada a um ficheiro fora da pasta da publicação, que o JeffPub não abre automaticamente. A página mostra a pequena pré-visualização, e é isso que seria impresso. | No Gestor de Gráficos, selecione a imagem e clique em **Atualizar Ligação**. |
| A imagem tem baixa resolução | A imagem tem menos de 150 ppi (pixels por polegada) no tamanho de impressão. | Utilize uma imagem maior ou imprima-a mais pequena. |
| O objeto tem transparência ou efeitos (pode ser impresso de forma diferente) | As áreas transparentes e os efeitos podem ter um aspeto diferente no papel. | Imprima uma página de teste. |
| O tipo de letra "nome" não está instalado | Um tipo de letra do texto não existe neste computador, pelo que é substituído por outro. | Instale o tipo de letra ou escolha outro. |

Cada mensagem termina com o número da página, por exemplo "(Página 2)".

## Antes de criar um ficheiro para a gráfica

Quando cria um ficheiro PDF/X, o JeffPub avisa-o sobre imagens de baixa resolução e tipos de letra em falta e propõe abrir o Verificador de Estrutura. Consulte [Ficheiros PDF e gráficas](pdf).

## Tópicos relacionados

- [Ficheiros PDF e gráficas](pdf)
- [Imprimir uma publicação](printing)
- [Fazer fluir texto entre caixas de texto](linked-text)
- [Adicionar e ajustar imagens](pictures)
- [Gestor de Gráficos](graphics-manager)
